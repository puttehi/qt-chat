#include "chatwindow.h"
#include "ui_chatwindow.h"
#include <QWebSocket>
#include <connectwindow.h>
#include <QTextBlock>
#include <keyeventtextedit.h>

#include <packets.h>
#include <shared.h>
#include <QCloseEvent>
#include <QMessageBox>
#include <QScrollBar>
#include <QTimeZone>
#include <QFileDialog>

ChatWindow::ChatWindow(ConnectWindow* connectWindow, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChatWindow)
{
    ui->setupUi(this);
    m_connectWindow = connectWindow;
    connect(ui->teMessage, &KeyEventTextEdit::submitted, this, &ChatWindow::ClearMessageBox);
}

ChatWindow::~ChatWindow()
{
    delete ui;
}

void ChatWindow::SetChatroom(QString &ip, QString &port, QString &name)
{
    this->setWindowTitle(ip + ":" + port + " as " + name);
}

void ChatWindow::ReceiveMessage(Shared::UserInfo &userInfo, QString &message, qint64 &millisSinceEpoch)
{
    if (showTimestamps){
        QLocale systemLocale(QLocale::system());
        QString dateTimeString = QString("[%1]").arg(systemLocale.toString(Shared::MakeTimeStamp(millisSinceEpoch), QLocale::FormatType::ShortFormat));
        ui->tbSentMessages->append(dateTimeString + " " + UserInfoToColorizedHTMLUserName(userInfo) + ": " + message);
    } else {
        ui->tbSentMessages->append(UserInfoToColorizedHTMLUserName(userInfo) + ": " + message);
    }
}

void ChatWindow::ReceiveMessage(QString &message, qint64 &millisSinceEpoch)
{
    if (showTimestamps){
        QLocale systemLocale(QLocale::system());
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QString dateTimeString = QString("[%1]").arg(systemLocale.toString(Shared::MakeTimeStamp(millisSinceEpoch), QLocale::FormatType::ShortFormat));
        ui->tbSentMessages->append(dateTimeString + " " + message);
    } else {
        ui->tbSentMessages->append(message);
    }
}

void ChatWindow::on_teMessage_textChanged()
{
    m_message = ui->teMessage->toPlainText();
}

void ChatWindow::AddUserToList(Shared::UserInfo &userInfo){
    ui->tbUserList->append(UserInfoToColorizedHTMLUserName(userInfo));
}

void ChatWindow::RebuildUserList(QList<Shared::UserInfo> userInfoList)
{
    ui->tbUserList->clear();
    for (auto userInfo : userInfoList){
        AddUserToList(userInfo);
    }
}


void ChatWindow::on_btnSend_clicked()
{
    ClearMessageBox();
    ui->teMessage->setFocus(); // focus typing box for ready for a new message
}

void ChatWindow::SendMessage(QString &message)
{
    using namespace Packet;
    if (m_connectWindow){
        QWebSocket* socket = m_connectWindow->GetSocket();
        if (socket != NULL){
            int sentBytes = socket->sendBinaryMessage(
                        BasePacket::PreparePacket(
                            new MessagePacket(message, m_connectWindow->GetUserInfo())));
            qDebug() << "Sent bytes(" << sentBytes << "): ";
        } else {
            qDebug() << "Socket was null!";
        }
    }

}

void ChatWindow::ClearMessageBox()
{
    SendMessage(m_message);
    ui->teMessage->clear();
}

void ChatWindow::closeEvent (QCloseEvent *event)
{
    QString title, info;
    if (m_connectWindow->IsServerAlive()){
        title = "Close chat room";
        info = QString("Are you sure you want to close the chat room?\n\n") +
               QString("This will kick all users and delete the chat history!");
    } else {
        title = "Close chat room";
        info = "Are you sure you want to close the chat room?\n";
    }
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, title,
                                                                info,
                                                                QMessageBox::Cancel | QMessageBox::Yes,
                                                                QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        event->ignore();
    } else {
        m_connectWindow->Disconnect();
        Reset();
        event->accept();
    }
}

void ChatWindow::Reset()
{
    m_message = "";
    ui->tbSentMessages->clear();
    ui->tbUserList->clear();
    ui->teMessage->clear();
}

void ChatWindow::SaveChatLog()
{
    QString dateTimeString = QDateTime::currentDateTimeUtc().toString(Qt::DateFormat::SystemLocaleShortDate);
    QString fileName = QFileDialog::getSaveFileName(this,
                tr("Save chat log"),
                dateTimeString,
                tr("Text file (*.txt)"));
        if (fileName.isEmpty()) {
            return;
        }
        else {
            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QMessageBox::information(this, tr("Unable to open file"),
                    file.errorString());
                return;
            }
            // write text to file:
            //file.write();
            QTextStream out(&file);
            QString text = ui->tbSentMessages->toPlainText();
            out << text;
            file.close();
         }
}

QColor ChatWindow::ToQColor(Shared::UserInfo::Color fromColor)
{
    switch (fromColor){
    case Shared::UserInfo::Color::BLACK:{
        return QColor("black");
        break;
    }
    case Shared::UserInfo::Color::RED:{
        return QColor("red");
        break;
    }
    case Shared::UserInfo::Color::GREEN:{
        return QColor("green");
        break;
    }
    case Shared::UserInfo::Color::BLUE:{
        //return QColor("blue"); // somewhat unreadable
        return QColor("darkblue");
        break;
    }
    case Shared::UserInfo::Color::MAGENTA:{
        return QColor("magenta");
        break;
    }
    case Shared::UserInfo::Color::CYAN:{
        //return QColor("cyan"); // unreadable
        return QColor("dodgerblue");
        break;
    }
    case Shared::UserInfo::Color::YELLOW:{
        //return QColor("yellow"); // unreadable
        return QColor("goldenrod");
        break;
    }
    default: {
        return QColor("black");
        break;
    }
    }
}

QString ChatWindow::UserInfoToColorizedHTMLUserName(Shared::UserInfo userInfo)
{
    QString colorName = ToQColor(userInfo.m_color).name();
    QString html = QString("<font color=\"%1\">%2</font>").arg(colorName, userInfo.m_userName);
    return html;
}

//QDateTime ChatWindow::MakeTimeStamp(qint64 millisSinceEpoch)
//{
//    QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(millisSinceEpoch);
//    return timestamp;
//}

void ChatWindow::on_cbTimestamps_stateChanged(int arg1)
{
    showTimestamps = arg1;
}

void ChatWindow::on_btnSaveLog_clicked()
{
    SaveChatLog();
}
