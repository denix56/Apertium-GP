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

/*!
  \class LangDelegate
  \ingroup gui
  \inmodule Apertium-GP
  \brief The delegate that is used for language combo boxes.
  */
#include <QPainter>

#include "langdelegate.h"

LangDelegate::LangDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}
/*!
 * \brief A reimplemented \l  {QItemDelegate::paint() const}, that paints focus rectangle for not empty vlaues.
 */
void LangDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    if (option.state & QStyle::State_HasFocus && !index.data().toString().isEmpty())
        painter->fillRect(option.rect, QColor(220, 220, 220));

    QItemDelegate::paint(painter, option, index);
}

/*!
 * \brief Removes standard focus rectangle.
 */
void LangDelegate::drawFocus(QPainter *, const QStyleOptionViewItem &, const QRect &) const
{}

