#ifndef KEYEVENTTEXTEDIT_H
#define KEYEVENTTEXTEDIT_H

#include <QTextEdit>
#include <QObject>
/*Custom event on pressing Return*/
class KeyEventTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    KeyEventTextEdit(QWidget* parent = nullptr);
    void keyPressEvent(QKeyEvent *event);
signals:
    void submitted();
};

#endif // KEYEVENTTEXTEDIT_H
