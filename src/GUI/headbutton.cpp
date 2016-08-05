/*
* Copyright (C) 2016, Denys Senkin <denisx9.0c@gmail.com>
*
* This file is part of apertium-gp
*
* apertium-gp is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* apertium-gp is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with apertium-gp.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "headbutton.h"
#include <QPaintEvent>
#include <QFontMetrics>

HeadButton::HeadButton(QWidget* parent):QPushButton(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setCheckable(true);
    setFixedSize(102,27);
    QFont font(this->font());
    font.setPointSize(11);
    setFont(font);
    wasClicked=false;
    once=true;
    //FIXME: is it needed?
    connect(this,&HeadButton::pressed,this,&HeadButton::denySameButtonClick2);
    connect(this,&HeadButton::toggled,this,&HeadButton::denySameButtonClick);
    connect(this,&HeadButton::toggled,this,&HeadButton::changeButtonColor);
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


void HeadButton::paintEvent(QPaintEvent *e)
{
    if(fontMetrics().width(text()) != lastFontWidth) {
        lastFontWidth = fontMetrics().width(text());
        setFixedSize(qMax(lastFontWidth+10,102),27);
    }
    QPushButton::paintEvent(e);
}

 void HeadButton::setText(const QString &text)
 {
     if (!text.isEmpty())
        setToolTip(text);
     QPushButton::setText(text);
 }
