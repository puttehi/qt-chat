#include "packets.h"
#include <QDebug>
#include <QDataStream>
#include <shared.h>
#include <QDateTime>
namespace Packet {
BasePacket::BasePacket()
{
    timestamp = QDateTime::currentMSecsSinceEpoch();
}
/*
Convert Packet to QByteArray for sending
*/
QByteArray BasePacket::PreparePacket(BasePacket *packet)
{
    switch(packet->packetType){
    case CONNECTED: {
        ConnectionPacket* identifiedPacket = (ConnectionPacket*)packet;
        QByteArray  arrBlock;
        QDataStream out(&arrBlock, QIODevice::WriteOnly);
        out.setVersion( QDataStream::Qt_5_15 );
        out << identifiedPacket->packetType;
        out << identifiedPacket->userName;
        return arrBlock;
        break;
    }
    case DISCONNECTED:{
        DisconnectionPacket* identifiedPacket = (DisconnectionPacket*)packet;
        QByteArray  arrBlock;
        QDataStream out(&arrBlock, QIODevice::WriteOnly);
        out.setVersion( QDataStream::Qt_5_15 );
        out << identifiedPacket->packetType;
        out << identifiedPacket->userInfo;
        return arrBlock;
        break;
    }
    case MESSAGE:{
        MessagePacket* identifiedPacket = (MessagePacket*)packet;
        QByteArray  arrBlock;
        QDataStream out(&arrBlock, QIODevice::WriteOnly);
        out.setVersion( QDataStream::Qt_5_15 );
        out << identifiedPacket->packetType;
        out << identifiedPacket->userInfo;
        out << identifiedPacket->message;
        return arrBlock;
        break;
    }
    case ANONYMOUS_MESSAGE:{
        AnonymousMessagePacket* identifiedPacket = (AnonymousMessagePacket*)packet;
        QByteArray  arrBlock;
        QDataStream out(&arrBlock, QIODevice::WriteOnly);
        out.setVersion( QDataStream::Qt_5_15 );
        out << identifiedPacket->packetType;
        out << identifiedPacket->message;
        return arrBlock;
        break;
    }
    case ONLINE_USERS:{
        OnlineUsersPacket* identifiedPacket = (OnlineUsersPacket*)packet;
        QByteArray  arrBlock;
        QDataStream out(&arrBlock, QIODevice::WriteOnly);
        out.setVersion( QDataStream::Qt_5_15 );
        out << identifiedPacket->packetType;
        out << identifiedPacket->userInfoList;
        return arrBlock;
        break;
    }
    case USER_INFO:{
        UserInfoPacket* identifiedPacket = (UserInfoPacket*)packet;
        QByteArray  arrBlock;
        QDataStream out(&arrBlock, QIODevice::WriteOnly);
        out.setVersion( QDataStream::Qt_5_15 );
        out << identifiedPacket->packetType;
        out << identifiedPacket->userInfo;
        return arrBlock;
        break;
    }
    case HISTORY:{
        MessageHistoryPacket* identifiedPacket = (MessageHistoryPacket*)packet;
        QByteArray  arrBlock;
        QDataStream out(&arrBlock, QIODevice::WriteOnly);
        out.setVersion( QDataStream::Qt_5_15 );
        out << identifiedPacket->packetType;
        out << identifiedPacket->timestampedMessagePairList;
        return arrBlock;
        break;
    }
    default:{
        return NULL;
        break;
    }
    }
}
/*
Figure out packet type and return the proper packet
*/
BasePacket *BasePacket::ReadPacket(QByteArray data)
{
    Type packetType;
    //qint64 timestamp; // millisSinceEpoch
    QDataStream in(&data, QIODevice::ReadOnly);
    in >> packetType;
    //in >> timestamp;
    qDebug() << "ReadPacket: type was " << packetType;
    if (packetType == Type::MESSAGE){
        Shared::UserInfo userInfo;
        QString message;
        in >> userInfo;
        in >> message;
        MessagePacket* asPacket = new MessagePacket(message, userInfo);
        return asPacket;
    } else if (packetType == Type::ANONYMOUS_MESSAGE){
        QString message;
        in >> message;
        AnonymousMessagePacket* asPacket = new AnonymousMessagePacket(message);
        return asPacket;
    } else if (packetType == Type::CONNECTED){
        QString userName;
        in >> userName;
        ConnectionPacket* asPacket = new ConnectionPacket(userName);
        return asPacket;
    } else if (packetType == Type::DISCONNECTED){
        Shared::UserInfo userInfo;
        in >> userInfo;
        DisconnectionPacket* asPacket = new DisconnectionPacket(userInfo);
        return asPacket;
    } else if (packetType == Type::ONLINE_USERS){
        QList<Shared::UserInfo> userList;
        in >> userList;
        OnlineUsersPacket* asPacket = new OnlineUsersPacket(userList);
        return asPacket;
    } else if (packetType == Type::USER_INFO) {
        Shared::UserInfo userInfo;
        in >> userInfo;
        UserInfoPacket* asPacket = new UserInfoPacket(userInfo);
        return asPacket;
    } else if (packetType == Type::HISTORY) {
        //QList<QPair<Shared::UserInfo, QString>> messagePairList;
        //in >> messagePairList;
        //MessageHistoryPacket* asPacket = new MessageHistoryPacket(messagePairList);
        QList<Shared::TimestampedMessage> messagePairList;
        in >> messagePairList;
        MessageHistoryPacket* asPacket = new MessageHistoryPacket(messagePairList);
        return asPacket;
    }
    return NULL;
}

ConnectionPacket::ConnectionPacket(QString _userName) : userName(_userName)/*userInfo(_userInfo)*/
{
    packetType = Type::CONNECTED;
}

ConnectionPacket::~ConnectionPacket()
{
}

DisconnectionPacket::DisconnectionPacket(Shared::UserInfo _userInfo) : userInfo(_userInfo)
{
    packetType = Type::DISCONNECTED;
}

DisconnectionPacket::~DisconnectionPacket()
{
}

OnlineUsersPacket::OnlineUsersPacket(QList<Shared::UserInfo> _userInfoList) : userInfoList(_userInfoList)
{
    packetType = Type::ONLINE_USERS;
}

OnlineUsersPacket::~OnlineUsersPacket()
{
}

MessagePacket::MessagePacket(QString _message, Shared::UserInfo _userInfo) : userInfo(_userInfo), message(_message)
{
    packetType = Type::MESSAGE;
}

MessagePacket::~MessagePacket()
{
}

UserInfoPacket::UserInfoPacket(Shared::UserInfo _userInfo) : userInfo(_userInfo)
{
    packetType = Type::USER_INFO;
}

UserInfoPacket::~UserInfoPacket()
{
}

//MessageHistoryPacket::MessageHistoryPacket(QList<QPair<Shared::UserInfo, QString> > _messagePairList) : messagePairList(_messagePairList)
MessageHistoryPacket::MessageHistoryPacket(QList<Shared::TimestampedMessage> _messagePairList) : timestampedMessagePairList(_messagePairList)
{
    packetType = Type::HISTORY;
}

MessageHistoryPacket::~MessageHistoryPacket()
{
}

AnonymousMessagePacket::AnonymousMessagePacket(QString _message) : message(_message)
{
    packetType = Type::ANONYMOUS_MESSAGE;
}

AnonymousMessagePacket::~AnonymousMessagePacket()
{
}

} //!namespace Packet
