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

#pragma once
#include <QPushButton>

class HeadButton : public QPushButton
{
    Q_OBJECT
public:
    HeadButton(QWidget *parent = 0);

    void setText(const QString &text);

protected:
    void paintEvent(QPaintEvent *);

public slots:
    void changeButtonColor(bool);

private slots:
    void denySameButtonClick();

    void denySameButtonClick2();

private:
    bool wasClicked;
    bool once;
    int lastFontWidth;
};

