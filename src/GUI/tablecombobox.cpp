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


#include <QHeaderView>
#include <QDebug>

#include "langdelegate.h"

#include "tablecombobox.h"

TableComboBox::TableComboBox(QWidget *parent)
    : QComboBox(parent)
{
    setView(new QTableView(this));
    setModel(new LanguageTableModel(view()));
    view()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view()->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);      //fine tuning of some options
    view()->setSelectionMode(QAbstractItemView::SingleSelection);
    view()->setSelectionBehavior(QAbstractItemView::SelectItems);
    view()->setFocusPolicy(Qt::StrongFocus);
    view()->setShowGrid(false);
    view()->setAutoScroll(false);
    view()->horizontalHeader()->hide();
    view()->verticalHeader()->hide();
    view()->setItemDelegate(new LangDelegate(view()));
    view()->setMouseTracking(true);
    setMaxVisibleItems(model()->rowCount());
    QFont font(this->font());
    font.setPointSize(11);
    setFont(font);
    connect(model(), &LanguageTableModel::dataChanged,
            this, &TableComboBox::resizeTable);
}

QTableView *TableComboBox::view() const
{
    return qobject_cast<QTableView *> (QComboBox::view());
}

LanguageTableModel *TableComboBox::model() const
{
    return qobject_cast<LanguageTableModel *>(QComboBox::model());
}

void TableComboBox::setView(QTableView *view)
{
    QComboBox::setView(view);
}

void TableComboBox::setModel(LanguageTableModel *model)
{
    QComboBox::setModel(model);
}

void TableComboBox::resizeTable()
{
    if (model()->itemCount() == 0)
        return;
    int colc = model()->columnCount();
    for (int i = 0; i < colc; i++)
        view()->setColumnWidth(i, COLWIDTH);
    view()->setMinimumWidth(COLWIDTH * (colc));
    view()->setMinimumHeight((model()->rowCount())*view()->rowHeight(0));
}

void TableComboBox::wheelEvent(QWheelEvent *)
{}
