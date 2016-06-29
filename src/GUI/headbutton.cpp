#include "headbutton.h"
#include <QPaintEvent>
#include <QFontMetrics>

HeadButton::HeadButton(QWidget* parent):QPushButton(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setCheckable(true);
    setFixedSize(102,27);
    wasClicked=false;
    once=true;
    //FIXME: is it needed?
    connect(this,&HeadButton::pressed,this,&HeadButton::denySameButtonClick2);
    connect(this,&HeadButton::toggled,this,&HeadButton::denySameButtonClick);

}

void HeadButton::denySameButtonClick()
{
    if(once) {
        if(wasClicked && !this->isChecked())
            wasClicked=false;
        else
            if(!wasClicked && this->isChecked())
                wasClicked=true;
    }
    once=!once;
}

void HeadButton::denySameButtonClick2()
{
    if(wasClicked)
        setChecked(false);
}
void HeadButton::changeButtonColor(bool checked)
{
    setDefault(checked);
}

HeadButton::~HeadButton(){}

void HeadButton::paintEvent(QPaintEvent *e)
{
    if(fontMetrics().width(text()) != lastFontWidth) {
        lastFontWidth = fontMetrics().width(text());
        setFixedSize(qMax(lastFontWidth+10,102),27);
    }
    QPushButton::paintEvent(e);
}

