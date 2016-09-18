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

#include <QAbstractTableModel>

class LanguageTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit LanguageTableModel(QObject *parent = 0);

    explicit LanguageTableModel(QStringList &list, QObject *parent = 0);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool addItem(const QVariant &value);

    bool removeItem(const int &row, const int &column);


    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    //set maximum number
    bool setNumberOfRows(int &n);


    //clear the model
    void clear();

    //count number of languages
    int itemCount() const;

    //return maximum number of elements that can be shown in a column
    int maxRowCount() const;

    //find position of language by its name
    QModelIndex findText(QString value) const;

private:
    //count number of languages in the current column
    int currentColumnRowCount(const QModelIndex &index);

    int rowN = 8;

    QStringList list;
};

