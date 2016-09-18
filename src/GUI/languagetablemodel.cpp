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
  \class LanguageTableModel
  \ingroup gui
  \inmodule Apertium-GP
  \brief This class provides a multi-column model for language combo boxes.

  Due to that \l QComboBox does not support multi-column model, it is a custom implementation.
 */

#include <QStringList>
#include <QDebug>

#include "languagetablemodel.h"

LanguageTableModel::LanguageTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{}

/*!
  Constructs \l LanguageTableModel and fills it with \a list values
 */
LanguageTableModel::LanguageTableModel(QStringList &list, QObject *parent)
    : QAbstractTableModel(parent), list(list)
{}

int LanguageTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return itemCount() < rowN ? list.size() : rowN;
}

int LanguageTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (list.size()==0)
        return 1;
    return list.size()/rowN + (list.size() % rowN == 0 ? 0 : 1);
}

QVariant LanguageTableModel::data(const QModelIndex &index, int role) const
{
    int pos = index.column()*rowN+index.row();
    if (!index.isValid() || pos>=list.size())
        return QVariant();
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
        auto ans = list.at(pos);
        return QVariant(ans);
    }
    return QVariant();
}

bool LanguageTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(role)
    int pos = index.column()*rowN + index.row();
    //if not in the end
    if (pos < list.size())
        list.replace(pos,value.toString());
    else
    {
        if(list.size() >= rowN) {
            if (index.row() == 0)
                beginInsertColumns(QModelIndex(),columnCount(),columnCount());
            list << value.toString();
            for (int i = index.row()+1; i<rowCount(); i++)
                list << "";
            if (index.row() == 0)
                endInsertColumns();
        }
        else {
            beginInsertRows(QModelIndex(),rowCount(),rowCount());
            list << value.toString();
            endInsertRows();
        }
    }
    //Alphabethic sorting
    QString text = tr("Identify language...");
    int i = list.indexOf(text);
    if (i!=-1)
        list.removeAt(i);
    qSort(list.begin(),list.begin()+itemCount());
    if (i!=-1)
        list.push_front(text);
    emit dataChanged(createIndex(0,0), createIndex(rowCount()-1,columnCount()-1), QVector<int>() << Qt::EditRole);
    return true;
}
int LanguageTableModel::currentColumnRowCount(const QModelIndex &index)
{
    if (columnCount() > index.column() + 1)
        return rowN;
    else
        for (int i = 0; i<rowCount(); ++i)
            if (list.at(rowN*index.column() + i)=="")
                return i;
    return 0;
}

bool LanguageTableModel::addItem(const QVariant &value)
{
    if (columnCount()>1 || list.size()==rowN) {
        int row = currentColumnRowCount(createIndex(0,columnCount()-1));
        if (row==rowCount())
            row = 0;
        int n = list.size();
        if (list.at(columnCount()*rowN-1)!="")
            ++n;
        return setData(createIndex(row, n/rowN + (n % rowN == 0 ? 0 : 1)-1),value);
    }
    else
        return setData(createIndex(list.size(), 0), value);
}

bool LanguageTableModel::removeItem(const int &row, const int &column)
{
    beginRemoveRows(QModelIndex(),row,row);
    list.removeAt(column*rowN + row);
    endRemoveRows();
    if (itemCount()>rowN) {
        beginInsertRows(QModelIndex(),rowCount(),rowCount());
        list << "";
        endInsertRows();
    }
    //remove empty column
    if (columnCount()>1 && !currentColumnRowCount(createIndex(0,columnCount()-1)))
    {
        beginRemoveColumns(QModelIndex(),columnCount()-1,columnCount()-1);
        list.erase(list.begin()+(columnCount()-1)*rowN, list.end());
        endRemoveColumns();
    }
    //Alphabethic sorting
    beginResetModel();
    qSort(list.begin(),list.begin()+itemCount());
    endResetModel();
    emit dataChanged(createIndex(0,0), createIndex(rowCount()-1,columnCount()-1), QVector<int>() << Qt::EditRole);
    return true;
}

Qt::ItemFlags LanguageTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled;
}

bool LanguageTableModel::setNumberOfRows(int &n)
{
    if (rowN<=0)
    {
        qDebug() << "Invalid number of rows";
        return false;
    }
    rowN = n;
    return true;
}

void LanguageTableModel::clear()
{
    beginResetModel();
    list.clear();
    endResetModel();
    emit dataChanged(createIndex(0,0),createIndex(rowCount(), columnCount()), QVector<int>() << Qt::EditRole);
}

QModelIndex LanguageTableModel::findText(QString value) const
{
    int pos = list.indexOf(value);
    return pos != -1 ? createIndex(pos % rowN, pos / rowN)
           : createIndex(-1,-1);
}

int LanguageTableModel::itemCount() const
{
    int count = 0;
    for (int i = 0; i < list.size(); i++)
    {
        if (list.at(i) == "")
            return count;
        count++;
    }
    return count;
}

int LanguageTableModel::maxRowCount() const
{
    return rowN;
}
