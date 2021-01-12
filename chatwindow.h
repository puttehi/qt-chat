#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QMainWindow>

QT_FORWARD_DECLARE_CLASS(QWebSocket)
QT_FORWARD_DECLARE_CLASS(Server)
QT_FORWARD_DECLARE_CLASS(ConnectWindow)

#include <shared.h>
#include <QTextEdit>
namespace Ui {
class ChatWindow;
}

class ChatWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit ChatWindow(ConnectWindow* connectWindow, QWidget *parent = nullptr);
    ~ChatWindow();

    void SetChatroom(QString &ip, QString &port, QString &name); // set window title
    void ReceiveMessage(Shared::UserInfo &userInfo, QString &message, qint64 &millisSinceEpoch); // append message to GUI
    void ReceiveMessage(QString &message, qint64 &millisSinceEpoch); // append anonymous message to GUI
    void AddUserToList(Shared::UserInfo &userInfo); // append user to GUI
    void RebuildUserList(QList<Shared::UserInfo> userInfoList); // clear user list and AddUserToList for each user in list
private slots:
    void on_teMessage_textChanged();
    void on_btnSend_clicked();
    void on_cbTimestamps_stateChanged(int arg1);

    void on_btnSaveLog_clicked();

private:
    Ui::ChatWindow *ui;

    ConnectWindow* m_connectWindow;

    QString m_message; // to be sent
    bool showTimestamps = true;

    void closeEvent(QCloseEvent *event); // hook close event for calling Disconnect() on close

    void SendMessage(QString &message); // send message from typing box to server
    void ClearMessageBox(); // clear text from typing box
    void Reset(); // reset params
    void SaveChatLog();

    QColor ToQColor(Shared::UserInfo::Color fromColor);
    QString UserInfoToColorizedHTMLUserName(Shared::UserInfo userInfo);
    //QDateTime MakeTimeStamp(qint64 millisSinceEpoch);
};

#endif // CHATWINDOW_H
