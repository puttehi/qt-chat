#ifndef SERVER_H
#define SERVER_H

#include <QObject>

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)
QT_FORWARD_DECLARE_CLASS(ConnectWindow)
QT_FORWARD_DECLARE_CLASS(ChatWindow)

#include <shared.h>
class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(ConnectWindow* connectWindow, int port, QObject *parent = nullptr);
    ~Server() override;
    enum Commands{
        SLAP, // /slap
        ME, // /me
        TIME // /time
    };
signals:
    void OnServerCreated(QString &ip, QString &port);
private slots:
    void newConnection();
    void userDisconnected();
    void processPacket(const QByteArray &bytes);
private:
    ChatWindow* m_chatWindow;

    // server data
    QWebSocketServer *m_wsServer;
    QList<QWebSocket*> m_clients;
    QList<Shared::UserInfo> m_users;

    //QList<QPair<Shared::UserInfo, QString>> messageHistory;
    QList<Shared::TimestampedMessage> messageHistory;

    // controlling clients
    void AddClient(QWebSocket* socket); // add socket to clients(web io)
    void RemoveClient(QWebSocket* socket); // remove socket from clients(web io)

    // controlling users
    Shared::UserInfo CreateUser(QString &_userName); // create a new userinfo for just connected user
    void AddUser(Shared::UserInfo &userInfo); // add user to users (user data)
    void RemoveUser(Shared::UserInfo &userInfo); // remove user from users (user data)

    // sending/receiving data to/from clients
    void SendUserlistToClients(); // send online users to clients
    void SendCreatedUserInfo(Shared::UserInfo userInfo, QWebSocket* recipient); // send user info to recipient
    void EchoReceivedMessage(Shared::UserInfo userInfo, QString message); // echo message back to all clients
    void EchoAnonymousMessage(QString message); // echo a "server message" to all clients (no userinfo)
    void SendChatHistory(QWebSocket* recipient); // send messageHistory (list of userinfo-message pairs) to recipient

    // server utilities
    void AppendMessagePairToHistory(QPair<Shared::UserInfo, QString> userMessagePair, qint64 millisSinceEpoch);

    // server commands (e.g. /slap)
    QString GetCommandString(Commands commandType); // return the command the enum points to (Commands::SLAP returns /slap)
    bool MessageCommand(Shared::UserInfo &senderUserInfo, QString &message); // server command recognition
    void Slap(QString &slapper, QString &slappee); // "<slapper> slaps <slappee> around a bit with a large trout"
    void Me(QString &me, QString &status); // "<me> <status>"
    void Time(); //"Current server time is: <QDateTime.currentDateTime().toString()>"
};

#endif // SERVER_H
