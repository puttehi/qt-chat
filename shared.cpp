#include "shared.h"
#include <QDataStream>
#include <QRandomGenerator>
#include <QDebug>

namespace Shared{
    const QList<UserInfo::Color> UserInfo::allColors = {
        UserInfo::Color::BLACK,
        UserInfo::Color::GREEN,
        UserInfo::Color::RED,
        UserInfo::Color::BLUE,
        UserInfo::Color::MAGENTA,
        UserInfo::Color::CYAN,
        UserInfo::Color::YELLOW
    };

    UserInfo::UserInfo(QString userName, UserInfo::Color color)
    {
       m_userName = userName;
       m_color = color;
    }

    UserInfo::UserInfo()
    {
        m_userName = "";
        m_color = Color::BLACK;
    }

    UserInfo UserInfo::MakeUserInfoRandomColor(QString &userName)
    {
        QRandomGenerator random(static_cast<long unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
        int randomIndex = random.bounded(allColors.length());
        Color randomColor = allColors[randomIndex];
        qDebug() << "Generated user info: " << userName << "(" << randomColor << ") with index " << randomIndex;
        return UserInfo(userName, randomColor);
    }
    QDataStream& operator<<(QDataStream &out, const UserInfo &userInfo){
        out << userInfo.m_userName << userInfo.m_color;
        return out;
    }
    QDataStream& operator>>(QDataStream &in, UserInfo &userInfo){
        in >> userInfo.m_userName >> userInfo.m_color;
        return in;
    }

    QPair<UserInfo, QString> MakeUserMessagePair(UserInfo userInfo, QString message)
    {
        QPair<Shared::UserInfo, QString> pair(userInfo, message);
        return pair;
    }
    QDateTime MakeTimeStamp(qint64 millisSinceEpoch)
    {
        QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(millisSinceEpoch);
        return timestamp;
    }

    QDataStream& operator<<(QDataStream &out, const TimestampedMessage &timestampedMessage){
        out << timestampedMessage.messagePair.first << timestampedMessage.messagePair.second << timestampedMessage.millisSinceEpoch;
        return out;
    }
    QDataStream& operator>>(QDataStream &in, TimestampedMessage &timestampedMessage){
        in >> timestampedMessage.messagePair.first >> timestampedMessage.messagePair.second >> timestampedMessage.millisSinceEpoch;
        return in;
    }
}
