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
#include <QItemDelegate>
class LangDelegate : public QItemDelegate
{
public:
    LangDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void drawFocus(QPainter *, const QStyleOptionViewItem &, const QRect &) const;
};

