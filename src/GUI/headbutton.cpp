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


#include <QPaintEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOptionButton>

#include "headbutton.h"

HeadButton::HeadButton(QWidget* parent)
    : QPushButton(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setCheckable(true);
    setMinimumSize(112,27);
    wasClicked = false;
    once = true;
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


void HeadButton::paintEvent(QPaintEvent *)
{
//    if(fontMetrics().width(text()) != lastFontWidth) {
//        lastFontWidth = fontMetrics().width(text());
//        setFixedSize(qMax(lastFontWidth+10,102),27);
//    }
    QStyleOptionButton option;
    initStyleOption(&option);;
    QRect textRect;

    QStyle* pStyle = style();
    if (pStyle != NULL)
    {
        QRect elementRect = pStyle->subElementRect(QStyle::SE_PushButtonContents, &option, this);
        int menuButtonSize = pStyle->pixelMetric(QStyle::PM_MenuButtonIndicator, &option, this);
        textRect = elementRect.adjusted(0, 0, -menuButtonSize, 0);
    }

       QString mElidedText = fontMetrics().elidedText(text(), Qt::ElideRight, textRect.width(), Qt::TextShowMnemonic);

    option.text = mElidedText;

    QStylePainter p(this);
 p.drawControl(QStyle::CE_PushButton, option);
}

 void HeadButton::setText(const QString &text)
 {
     if (!text.isEmpty())
        setToolTip(text);
     QPushButton::setText(text);
 }
