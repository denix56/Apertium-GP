#include "trayinputextedit.h"
#include <QKeyEvent>
#include <QClipboard>
#include <QApplication>
TrayInpuTextEdit::TrayInpuTextEdit(QWidget* parent)
    :InputTextEdit(parent)
{

}

void TrayInpuTextEdit::keyPressEvent(QKeyEvent *e)
{
    if(e->key()==Qt::Key_Return || e->key()==Qt::Key_Enter)
        return;
    if (e->matches(QKeySequence::Paste)) {
        QString text = QApplication::clipboard()->text();
        setPlainText(text.left(text.indexOf('\n')));
        return;
    }
    InputTextEdit::keyPressEvent(e);
}
