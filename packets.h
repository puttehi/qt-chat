#ifndef PACKETS_H
#define PACKETS_H
#include <QString>
#include <QList>
#include <QPair>
#include <shared.h>
/*
Packet Types
*/
namespace Packet {
enum Type{
    CONNECTED, // user connected
    DISCONNECTED, // user disconnected
    ONLINE_USERS, // echo server user list to users
    MESSAGE, // send chat message
    ANONYMOUS_MESSAGE, // send chat message without a sender ("server message")
    USER_INFO, // constructed UserInfo is echoed back to client on connection
    HISTORY,
};
/*
BASE PACKET
*/
class BasePacket
{    
public:
    BasePacket();
    virtual ~BasePacket() = default;
    static QByteArray PreparePacket(BasePacket* packet);
    static BasePacket* ReadPacket(QByteArray data);
    Type packetType; // packet is casted to correct class depending on type
    qint64 timestamp; // millis since epoch
};
/*
CONNECTION PACKET, Server decides color so only name is passed
*/
class ConnectionPacket : public BasePacket
{
public:
    ConnectionPacket(QString _userName);
    ~ConnectionPacket();
    QString userName;
};
/*
DISCONNECTION
*/
class DisconnectionPacket : public BasePacket
{
public:
    DisconnectionPacket(Shared::UserInfo _userInfo);
    ~DisconnectionPacket();
    Shared::UserInfo userInfo;
};
/*
USER LIST PACKET
*/
class OnlineUsersPacket : public BasePacket
{
public:
    OnlineUsersPacket(QList<Shared::UserInfo> _userInfoList);
    ~OnlineUsersPacket();
    QList<Shared::UserInfo> userInfoList;
};
/*
CHAT MESSAGE PACKET
*/
class MessagePacket : public BasePacket
{
public:
    MessagePacket(QString _message, Shared::UserInfo _userInfo);
    ~MessagePacket();
    Shared::UserInfo userInfo;
    QString message;
};
/*
ANONYMOUS CHAT MESSAGE PACKET
*/
class AnonymousMessagePacket : public BasePacket
{
public:
    AnonymousMessagePacket(QString _message);
    ~AnonymousMessagePacket();
    QString message;
};
/*
USERINFO PACKET
*/
class UserInfoPacket : public BasePacket
{
public:
    UserInfoPacket(Shared::UserInfo _userInfo);
    ~UserInfoPacket();
    Shared::UserInfo userInfo;
};
/*
CHAT HISTORY PACKET
*/
class MessageHistoryPacket : public BasePacket
{
public:
    //MessageHistoryPacket(QList<QPair<Shared::UserInfo, QString>> _messagePairList);
    MessageHistoryPacket(QList<Shared::TimestampedMessage> _messagePairList);
    ~MessageHistoryPacket();
    //QList<QPair<Shared::UserInfo, QString>> messagePairList;
    QList<Shared::TimestampedMessage> timestampedMessagePairList;
};
} //!namespace Packet

#endif // PACKETS_H
