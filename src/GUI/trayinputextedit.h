#ifndef TRAYINPUTEXTEDIT_H
#define TRAYINPUTEXTEDIT_H
#include "inputtextedit.h"

class TrayInpuTextEdit : public InputTextEdit
{
    Q_OBJECT
public:
    TrayInpuTextEdit(QWidget *parent = 0);
protected:
    void keyPressEvent(QKeyEvent *e);
};

#endif // TRAYINPUTEXTEDIT_H
