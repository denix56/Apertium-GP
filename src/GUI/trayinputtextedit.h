#ifndef TRAYINPUTEXTEDIT_H
#define TRAYINPUTEXTEDIT_H
#include "inputtextedit.h"

class TrayInputTextEdit : public InputTextEdit
{
    Q_OBJECT
public:
    TrayInputTextEdit(QWidget *parent = 0);
signals:
    void printEnded(QString text);
protected:
    void keyPressEvent(QKeyEvent *e);
};

#endif // TRAYINPUTEXTEDIT_H
