#include "connectwindow.h"
#include "ui_connectwindow.h"

#include <QWebSocket>
#include <QAbstractSocket>
#include <QUrl>

#include <packets.h>
#include <shared.h>

ConnectWindow::ConnectWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ConnectWindow)
{
    ui->setupUi(this);
    m_chatWindow = new ChatWindow(this);
    m_name = ui->leName->text();
    m_ip = ui->leIP->text();
    m_port = ui->lePort->text();
    socket = CreateSocket();
    server = NULL;
}

ConnectWindow::~ConnectWindow()
{
    socket->close();
    delete ui;
}

bool ConnectWindow::IsServerAlive()
{
    return server != NULL;
}

QWebSocket* ConnectWindow::GetSocket()
{
    return socket;
}

ChatWindow *ConnectWindow::GetChatWindow()
{
    return m_chatWindow;
}

void ConnectWindow::on_leName_textChanged(const QString &arg1)
{
    m_name = arg1;
}

void ConnectWindow::on_leIP_textChanged(const QString &arg1)
{
    m_ip = arg1;
}

void ConnectWindow::on_lePort_textChanged(const QString &arg1)
{
    m_port = arg1;
}

void ConnectWindow::on_btnConnect_clicked()
{
    ui->statusbar->showMessage("Connecting to chatroom " + m_ip + ":" + m_port + " as " + m_name + "...");
    Connect(m_ip, m_port, m_name);
}

void ConnectWindow::onConnected()
{
    using namespace Packet;
    this->hide();
    m_chatWindow->SetChatroom(m_ip,m_port,m_name);
    m_chatWindow->show();
    connect(socket, &QWebSocket::binaryMessageReceived, this, &ConnectWindow::processPacket);
    int sentBytes = socket->sendBinaryMessage(
               BasePacket::PreparePacket(
                    new ConnectionPacket(m_name)));
    qDebug() << "Sent bytes(" << sentBytes << "): ";
}

void ConnectWindow::onDisconnected() // called when socket is disconnected (socket->close())
{
    m_chatWindow->hide();
    this->show();
}

void ConnectWindow::onWebSocketError(QAbstractSocket::SocketError error)
{
    // Handle error here...
    if (socket) {
        qDebug() << socket->errorString();
    }
    qDebug() << error;
}

void ConnectWindow::processPacket(const QByteArray &bytes)
{
    qDebug() << "Client: Received packet";
    using namespace Packet;
    BasePacket* basePacket = BasePacket::ReadPacket(bytes);
    switch (basePacket->packetType){
    case MESSAGE:{
        // Message is a chat message with userinfo and the message as a string
        // Cast packet to correct type, send data to chat window
        MessagePacket* identifiedPacket = (MessagePacket*)basePacket;
        qDebug() << "Client: Message received " << identifiedPacket->userInfo /*identifiedPacket->userName;*/ << ": " << identifiedPacket->message;//identifiedPacket->userName << ": " << identifiedPacket->message;
        m_chatWindow->ReceiveMessage(identifiedPacket->userInfo, identifiedPacket->message, identifiedPacket->timestamp);
        break;
    }
    case ANONYMOUS_MESSAGE:{
        // Anonymoys message is just like a message but without a sender (no username coloring / "server message")
        // Cast packet to correct type, send data to chat window
        AnonymousMessagePacket* identifiedPacket = (AnonymousMessagePacket*)basePacket;
        qDebug() << "Client: Anonymous message received " << identifiedPacket->message;
        m_chatWindow->ReceiveMessage(identifiedPacket->message, identifiedPacket->timestamp);
        break;
    }
    case ONLINE_USERS:{
        // Online users contain a list of online userinfos
        // Cast packet to correct type, send data to chat window
        OnlineUsersPacket* identifiedPacket = (OnlineUsersPacket*)basePacket;
        m_chatWindow->RebuildUserList(identifiedPacket->userInfoList);
        break;
    }
    case USER_INFO:{
        // User info contains newly connected clients user info "echoed" back from the server on connection (server determines the color for the user)
        // Cast packet to correct type, set user info for this client
        UserInfoPacket* identifiedPacket = (UserInfoPacket*)basePacket;
        qDebug() << "Client: User info received " << identifiedPacket->userInfo;
        m_userInfo = identifiedPacket->userInfo;
        break;
    }
    case HISTORY:{
        // History contains the entire chat rooms chat history as a list of userinfo/message pairs
        // Cast packet to correct type, send data to chat window
        MessageHistoryPacket* identifiedPacket = (MessageHistoryPacket*)basePacket;
        //QList<QPair<Shared::UserInfo, QString>> messagePairList = identifiedPacket->messagePairList;
        QList<Shared::TimestampedMessage> timestampedMessagePairList = identifiedPacket->timestampedMessagePairList;
        qDebug() << "Client: Chat history received (message count: " << timestampedMessagePairList.count() << ")";
        // post each message to ui
        for (auto timestampedMessage : timestampedMessagePairList){
            Shared::UserInfo userInfo = timestampedMessage.messagePair.first;
            QString message = timestampedMessage.messagePair.second;
            qint64 timestamp = timestampedMessage.millisSinceEpoch;
            //m_chatWindow->ReceiveMessage(userInfo, message, identifiedPacket->timestamp);
            m_chatWindow->ReceiveMessage(userInfo, message, timestamp);
        }
        break;
    }
    default:{
        // Packet types like CONNECTED and DISCONNECTED are unhandled by the client as they exist only to inform the server
        qDebug() << "Client: Packet type was unhandled";
        break;
    }
    }
    //if (basePacket->packetType == Type::MESSAGE){
    //    MessagePacket* identifiedPacket = (MessagePacket*)basePacket;
    //    qDebug() << "Client: Message received " << identifiedPacket->userInfo /*identifiedPacket->userName;*/ << ": " << identifiedPacket->message;//identifiedPacket->userName << ": " << identifiedPacket->message;
    //    m_chatWindow->ReceiveMessage(identifiedPacket->userInfo, identifiedPacket->message, identifiedPacket->timestamp);
    //} else if (basePacket->packetType == Type::ANONYMOUS_MESSAGE){
    //    AnonymousMessagePacket* identifiedPacket = (AnonymousMessagePacket*)basePacket;
    //    qDebug() << "Client: Anonymous message received " << identifiedPacket->message;
    //    m_chatWindow->ReceiveMessage(identifiedPacket->message, identifiedPacket->timestamp);
    //} else if (basePacket->packetType == Type::ONLINE_USERS) {
    //    OnlineUsersPacket* identifiedPacket = (OnlineUsersPacket*)basePacket;
    //    m_chatWindow->RebuildUserList(identifiedPacket->userInfoList);
    //} else if (basePacket->packetType == Type::USER_INFO){
    //    UserInfoPacket* identifiedPacket = (UserInfoPacket*)basePacket;
    //    qDebug() << "Client: User info received " << identifiedPacket->userInfo;
    //    m_userInfo = identifiedPacket->userInfo;
    //} else if (basePacket->packetType == Type::HISTORY){
    //    MessageHistoryPacket* identifiedPacket = (MessageHistoryPacket*)basePacket;
    //    QList<QPair<Shared::UserInfo, QString>> messagePairList = identifiedPacket->messagePairList;
    //    qDebug() << "Client: Chat history received (message count: " << messagePairList.count() << ")";
    //    // post each message to ui
    //    for (auto messagePair : messagePairList){
    //        Shared::UserInfo userInfo = messagePair.first;
    //        QString message = messagePair.second;
    //        m_chatWindow->ReceiveMessage(userInfo, message, identifiedPacket->timestamp);
    //    }
    //}
}

void ConnectWindow::Connect(QString &ip, QString &port, QString &name)
{
    if (socket != NULL) {
        delete socket;
        socket = NULL;
    }
    socket = CreateSocket();

    // connect to server
    m_ip = ip;
    m_port = port;
    m_name = name;

    if (m_ip == "0.0.0.0") {
        m_ip = "localhost";
    }

    QUrl url("ws://" + m_ip + ":" + m_port);
    qDebug() << "Connecting to " << url << " (" << url.toString() << ")";
    socket->open(url);

    if (socket->state() != QAbstractSocket::ConnectingState){
        ui->statusbar->showMessage("Connecting to " + m_ip + ":" + m_port + "...");
    } else if (socket->state() == QAbstractSocket::ConnectedState){
        ui->statusbar->showMessage("Connected!");
    } else {
        // if connection (probably) failed
        ui->statusbar->showMessage("Connection failed!");
    }
}

void ConnectWindow::Disconnect()
{
    using namespace Packet;
    qDebug() << "Client: Closing connection...";
    if (socket){
        qDebug() << "Client: Sending disconnection packet...";
        int sentBytes = socket->sendBinaryMessage(
                BasePacket::PreparePacket(
                    new DisconnectionPacket(m_userInfo)));
    }
    if (socket && socket->state() != QAbstractSocket::UnconnectedState) {
        socket->close();
    }
    if (socket->state() == QAbstractSocket::ConnectedState){
        qDebug() << "Client: ERROR: Connection was not closed";
    } else if (socket->state() == QAbstractSocket::UnconnectedState) {
        qDebug() << "Client: Connection closed succesfully";
    }
    if (IsServerAlive()) {
        qDebug() << "Client: Client was also the server, closing server..";
        delete server;
        server = NULL;
    }
}

Shared::UserInfo ConnectWindow::GetUserInfo()
{
    return m_userInfo;
}

void ConnectWindow::Host(QString &port)
{
    m_port = port;
    if (QRegExp("\\d*").exactMatch(m_port)){
        int iPort = m_port.toInt();
        server = new Server(this, iPort);
    } else {
        ui->statusbar->showMessage("Error: Invalid port");
    }
}

QWebSocket *ConnectWindow::CreateSocket()
{
    QWebSocket* newSocket = new QWebSocket;
    connect(newSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &ConnectWindow::onWebSocketError);
    connect(newSocket, &QWebSocket::connected, this, &ConnectWindow::onConnected);
    connect(newSocket, &QWebSocket::disconnected, this, &ConnectWindow::onDisconnected);
    return newSocket;
}

void ConnectWindow::on_btnHost_clicked()
{
    ui->statusbar->showMessage("Creating a new chatroom...");
    Host(m_port);
}

void ConnectWindow::ServerCreated(QString &ip, QString &port)
{
    Connect(ip, port, m_name);
}


