#include "trayinputtextedit.h"
#include <QKeyEvent>
#include <QClipboard>
#include <QApplication>
TrayInputTextEdit::TrayInputTextEdit(QWidget* parent)
    :InputTextEdit(parent)
{
    connect(qobject_cast<InputTextEdit *>(this),&InputTextEdit::printEnded,[&]()
    {
        emit printEnded(this->toPlainText());
    });
}

void TrayInputTextEdit::keyPressEvent(QKeyEvent *e)
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
