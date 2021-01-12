#include "keyeventtextedit.h"
#include <QKeyEvent>
KeyEventTextEdit::KeyEventTextEdit(QWidget* parent) : QTextEdit(parent)
{

}

void KeyEventTextEdit::keyPressEvent(QKeyEvent *event)
   {
       if (event->key() == Qt::Key_Return)
       {
           emit(submitted());
       }
       else
       {
           QTextEdit::keyPressEvent(event);
       }
   }
