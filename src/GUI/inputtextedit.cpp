#include "inputtextedit.h"
#include <QKeyEvent>
InputTextEdit::InputTextEdit(QWidget* parent) :
    QTextEdit(parent)
{
    connect(&timer,&QTimer::timeout,this,&InputTextEdit::printEnded);
    connect(this,&InputTextEdit::textChanged,[&](){timer.start(250);});
    timer.setSingleShot(true);
}

