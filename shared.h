#ifndef SHARED_H
#define SHARED_H

#include <QString>
#include <QList>
#include <QDateTime>
#include <QDataStream>

namespace Shared { //TODO: Figure out why namespacing packets.h doesn't work
    class UserInfo {
    public:
        enum Color {
            BLACK,
            GREEN,
            RED,
            BLUE,
            MAGENTA,
            CYAN,
            YELLOW
        };
        UserInfo(QString userName, Color color);
        UserInfo();
        ~UserInfo() = default;
        static UserInfo MakeUserInfoRandomColor(QString &userName); // Create UserInfo with random color
        static const QList<Color> allColors;
        // QDataStream operator overloads
        friend QDataStream &operator<<(QDataStream &out, const UserInfo &userInfo);
        friend QDataStream &operator>>(QDataStream &in, UserInfo &userInfo);
        // qDebug operator overload (QString()-trick)
        operator QString() const { return "Username: " + m_userName + " (" + QString::number(m_color) + ")";}
        QString m_userName;
        Color m_color;
};

struct TimestampedMessage {
    TimestampedMessage(){}
    TimestampedMessage(QPair<Shared::UserInfo, QString> _messagePair, qint64 _millisSinceEpoch) : messagePair(_messagePair), millisSinceEpoch(_millisSinceEpoch){}
    QPair<Shared::UserInfo, QString> messagePair;
    qint64 millisSinceEpoch;

    friend QDataStream &operator<<(QDataStream &out, const TimestampedMessage &timestampedMessage);
    friend QDataStream &operator>>(QDataStream &in, TimestampedMessage &userInfo);
};

QPair<Shared::UserInfo, QString> MakeUserMessagePair(Shared::UserInfo userInfo, QString message);
QDateTime MakeTimeStamp(qint64 millisSinceEpoch);
}

#endif // SHARED_H
