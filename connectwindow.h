#ifndef CONNECTWINDOW_H
#define CONNECTWINDOW_H

#include <QMainWindow>
#include <chatwindow.h>
#include <server.h>
#include <QAbstractSocket>


namespace Ui {
class ConnectWindow;
}

class ConnectWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ConnectWindow(QWidget *parent = nullptr);
    ~ConnectWindow();
    bool IsServerAlive();
    QWebSocket* GetSocket();
    ChatWindow* GetChatWindow();
    void Disconnect();
    Shared::UserInfo GetUserInfo();
public slots:
    void ServerCreated(QString &ip, QString &port);
private slots:
    void on_leIP_textChanged(const QString &arg1);
    void on_lePort_textChanged(const QString &arg1);
    void on_btnConnect_clicked();
    void onConnected();
    void onDisconnected();
    void onWebSocketError(QAbstractSocket::SocketError error);
    void on_leName_textChanged(const QString &arg1);
    void on_btnHost_clicked();
    void processPacket(const QByteArray &bytes);
private:
    Ui::ConnectWindow *ui;
    ChatWindow *m_chatWindow;
    QString m_name;
    QString m_ip;
    QString m_port;
    Shared::UserInfo m_userInfo; // user info assigned from server after connecting
    Server* server; // server instance

    QWebSocket* socket; // connecting socket

    void Connect(QString &ip, QString &port, QString &name);
    void Host(QString &port);

    QWebSocket* CreateSocket();
};

#endif // CONNECTWINDOW_H
