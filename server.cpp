#include "server.h"

#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>

#include <cstdio>

#include <connectwindow.h>

#include <packets.h>
#include <shared.h>

#include <QLocale>
#include <QTimeZone>

Server::Server(ConnectWindow* connectWindow, int port, QObject *parent) : QObject(parent), m_wsServer(new QWebSocketServer(QStringLiteral("Chat Server"), QWebSocketServer::NonSecureMode, this))
{
    m_chatWindow = connectWindow->GetChatWindow();
    if (m_wsServer->listen(QHostAddress::Any, port))
    {
        QTextStream(stdout) << "Chat Server listening on port " << port << '\n';
        connect(m_wsServer, &QWebSocketServer::newConnection, this, &Server::newConnection);
        connect(this, &Server::OnServerCreated, connectWindow, &ConnectWindow::ServerCreated);
        QString ip = m_wsServer->serverAddress().toString();
        QString port = QString::number(m_wsServer->serverPort());
        qDebug() << "Server: created server!";
        emit(OnServerCreated(ip, port));
    }
}

Server::~Server()
{
    qDebug() << "Server: Closing server socket...";
    m_wsServer->close();
    qDebug() << "Server: Closed WebSocketServer and deleted server instance!";
}

void Server::newConnection()
{
    QWebSocket* socket = m_wsServer->nextPendingConnection();
    socket->setParent(this);
    connect(socket, &QWebSocket::binaryMessageReceived, this, &Server::processPacket);
    connect(socket, &QWebSocket::disconnected, this, &Server::userDisconnected);
    AddClient(socket);
}

void Server::userDisconnected()
{
    RemoveClient(qobject_cast<QWebSocket *>(sender()));
}

void Server::processPacket(const QByteArray &bytes)
{
    qDebug() << "Server: Packet received";
    using namespace Packet;
    BasePacket* basePacket = BasePacket::ReadPacket(bytes);
    if (basePacket->packetType == Type::DISCONNECTED){
        DisconnectionPacket* identifiedPacket = (DisconnectionPacket*)basePacket;
        qDebug() << "Server: Disconnection received " << identifiedPacket->userInfo;
        RemoveUser(identifiedPacket->userInfo);
        SendUserlistToClients();
    } else
    if (basePacket->packetType == Type::CONNECTED){
        ConnectionPacket* identifiedPacket = (ConnectionPacket*)basePacket;
        QWebSocket* sock = qobject_cast<QWebSocket *>(sender());
        qDebug() << "Server: Connection received " << identifiedPacket->userName;
        Shared::UserInfo userInfo = CreateUser(identifiedPacket->userName);
        AddUser(userInfo);
        SendCreatedUserInfo(userInfo, sock);
        SendChatHistory(sock);
    } else
    if (basePacket->packetType == Type::MESSAGE){
        MessagePacket* identifiedPacket = (MessagePacket*)basePacket;
        qDebug() << "Server: Message received " << identifiedPacket->userInfo << ": " << identifiedPacket->message;
        //TODO: make command packet type that is built by the client instead of command recognition (faster and more maintainable)
        if (MessageCommand(identifiedPacket->userInfo, identifiedPacket->message)){
            // message was command and was executed by it's handler (e.g. Slap())
        } else {
            // message was a normal chat message, no command logic executed
            EchoReceivedMessage(identifiedPacket->userInfo, identifiedPacket->message);
            AppendMessagePairToHistory(Shared::MakeUserMessagePair(identifiedPacket->userInfo, identifiedPacket->message), identifiedPacket->timestamp);
        }
    } else {
        qDebug() << "Server: Unknown packet type received, ignoring";
    }
}

void Server::AddClient(QWebSocket *socket)
{
    m_clients.append(socket);
}

void Server::RemoveClient(QWebSocket *socket)
{
    if (socket)
    {
        m_clients.removeAll(socket);
        socket->deleteLater();
    }
}

Shared::UserInfo Server::CreateUser(QString &_userName)
{
    return Shared::UserInfo::MakeUserInfoRandomColor(_userName);
}

void Server::AddUser(Shared::UserInfo &userInfo)
{
    m_users.append(userInfo);
    SendUserlistToClients();
}

void Server::RemoveUser(Shared::UserInfo &userInfo)
{
    m_users.removeOne(userInfo);
}

void Server::SendUserlistToClients()
{
    using namespace Packet;
    for (auto client : m_clients){
    int sentBytes = client->sendBinaryMessage(
                BasePacket::PreparePacket(
                    new OnlineUsersPacket(m_users)));
    }
}
void Server::SendCreatedUserInfo(Shared::UserInfo userInfo, QWebSocket *recipient)
{
    using namespace Packet;
    recipient->sendBinaryMessage(
                BasePacket::PreparePacket(
                    new UserInfoPacket(userInfo)));
}
void Server::EchoReceivedMessage(Shared::UserInfo userInfo, QString message)
{
    using namespace Packet;
    for (QWebSocket* client : qAsConst(m_clients)) {
        int sentBytes = client->sendBinaryMessage(
                    BasePacket::PreparePacket(
                        new MessagePacket(message, userInfo)));
    }
}

void Server::EchoAnonymousMessage(QString message)
{
    using namespace Packet;
    for (QWebSocket* client : qAsConst(m_clients)) {
        int sentBytes = client->sendBinaryMessage(
                    BasePacket::PreparePacket(
                        new AnonymousMessagePacket(message)));
    }
}

void Server::SendChatHistory(QWebSocket *recipient)
{
    using namespace Packet;
    recipient->sendBinaryMessage(
                BasePacket::PreparePacket(
                    new MessageHistoryPacket(messageHistory)));
}

void Server::AppendMessagePairToHistory(QPair<Shared::UserInfo, QString> userMessagePair, qint64 millisSinceEpoch)
{
    // Add timestamp to message before appending
    //QDateTime timestamp = Shared::MakeTimeStamp(millisSinceEpoch);
    messageHistory.append(Shared::TimestampedMessage(userMessagePair, millisSinceEpoch));
}

// Return string presentation of commandType
QString Server::GetCommandString(Server::Commands commandType)
{
    switch (commandType){
        case Commands::SLAP: {
            return QString("/slap");
            break;
        }
        case Commands::ME: {
            return QString("/me");
            break;
        }
        case Commands::TIME: {
            return QString("/time");
            break;
        }
    }
}

bool Server::MessageCommand(Shared::UserInfo &senderUserInfo, QString &message)
{
    // there's something to check?
    if (message.length() > 0){
        // there is, is it a command?
        if (message.at(0) == '/'){
            // it seems to be, what command is it?

            // create substring of start -> first whitespace to get command and use the rest after the whitespace as the command argument:
            int whiteSpaceIndex = message.indexOf(" "); // "/slap username" -> index is 5
            int arg1EndIndex = message.indexOf(" ", whiteSpaceIndex + 1); // "/kick user reason" -> index is 10
            //if (arg1EndIndex == -1) {
            //    // only single argument (last character was EOS)
            //    arg1EndIndex = message.length() - 1; // "/slap username" -> index is 13
            //}
            QString command = message.left(whiteSpaceIndex); // "/slap username" -> "/slap"
            if (command.compare(GetCommandString(Commands::SLAP)) == 0){
                QString arg1 = message.mid(whiteSpaceIndex + 1, (arg1EndIndex == -1 ? message.length() - 1 : arg1EndIndex) - whiteSpaceIndex); // "/slap username" -> "username"
                Slap(senderUserInfo.m_userName, arg1);
            } else if (command.compare(GetCommandString(Commands::ME)) == 0){
                QString arg1 = message.mid(whiteSpaceIndex + 1, message.length() - whiteSpaceIndex + 1); // "/me is programming" -> "is programming"
                Me(senderUserInfo.m_userName, arg1);
            } else if (command.compare(GetCommandString(Commands::TIME)) == 0){
                Time();
            }
            return true; // message was a command
        }
    }
    return false; // message was not a command
}

void Server::Slap(QString &slapper, QString &slappee)
{
    //TODO: Figure out a way to get the slappee sent as an userinfo for colorizing it in the users UI
    //NOTE: This might be a bad idea/confusing to the user since users might have identical names and the color might be wrong,
    //      maybe the command should be issued from a context menu instead or users should not be allowed to have identical names at all?
    //      For now, using strings only.
    QString message = QString("%1 slaps %2 around a bit with a large trout").arg(slapper, slappee);
    EchoAnonymousMessage(message);
    //TODO: Architecture for sending anonymous messages with the chat message history? (Chat history expects pairs of userinfo + string)
}

void Server::Me(QString &me, QString &status)
{
    QString message = QString("%1 %2").arg(me, status);
    EchoAnonymousMessage(message);
}

void Server::Time()
{
    QLocale systemLocale(QLocale::system());
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString timeZoneString = currentDateTime.timeZone().displayName(QTimeZone::TimeType::GenericTime, QTimeZone::NameType::OffsetName, systemLocale);
    QString dateTimeString = systemLocale.toString(currentDateTime, QLocale::FormatType::ShortFormat) + " (" + timeZoneString + ")";
    QString message = QString("%1").arg(dateTimeString);
    EchoAnonymousMessage(message);
}


