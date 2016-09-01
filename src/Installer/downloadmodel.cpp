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

#include <QDebug>
#include <QSize>

#include "downloadmodel.h"

DownloadModel::DownloadModel(QObject *parent)
    : QAbstractTableModel(parent)
{}

QVariant DownloadModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation==Qt::Horizontal && role == Qt::DisplayRole)
        switch(static_cast<Columns>(section)) {
        case Columns::NAME:
            return tr("Name");
        case Columns::TYPE:
            return tr("Type");
        case Columns::SIZE:
            return tr("Size");
        case Columns::STATE:
            return tr("Action");
        default:
            return QVariant();
        }
    return QVariant();
}

int DownloadModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return downList.size();
}

int DownloadModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 4;
}

QVariant DownloadModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role == Qt::DisplayRole) {
        int i = index.row();
        switch(static_cast<Columns>(index.column())) {

        case Columns::NAME:
            if (downList[i].type == Types::LANGPAIRS)
                return nameToFull(downList[i].name);
            else
                return downList[i].name;

        case Columns::TYPE:
            return typeNames[downList[i].type];

        case Columns::SIZE:
            return this->formatBytes(downList[i].size);

        case Columns::STATE:
            return stateNames[downList[i].state];
        }
    }
    return QVariant();
}

void DownloadModel::reset()
{
    beginResetModel();
    downList.clear();
    endResetModel();
}

bool DownloadModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        int i = index.row();
        switch(static_cast<Columns>(index.column())) {
        case Columns::NAME:
            downList[i].name = value.toString();
            break;
        case Columns::TYPE:
            downList[i].type = value.value<Types>();
            break;
        case Columns::SIZE:
            if (downList[i].state == States::DOWNLOADING)
                downList[i].progress = value.toInt();
            else
                downList[i].size = value.toUInt();
            break;
        case Columns::STATE:
            downList[i].state = value.value<States>();
            break;
        }
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

bool DownloadModel::addItem(const PkgInfo &f)
{
    beginInsertRows(QModelIndex(),rowCount(),rowCount());
    downList.push_back(f);
    endInsertRows();

    emit dataChanged(createIndex(rowCount()-1,0),createIndex(rowCount()-1,columnCount()-1),
                     QVector<int>() << Qt::EditRole);
    return true;
}

const PkgInfo *DownloadModel::item(const int &row) const
{
    if (row<downList.size())
        return &downList[row];
    else
        return nullptr;
}

Qt::ItemFlags DownloadModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled;
}

void DownloadModel::sort(int column, Qt::SortOrder order)
{
    switch (static_cast<Columns>(column))
    {
    //sort by names
    case Columns::NAME:
        qSort(downList.begin(),downList.end(),[&](const PkgInfo &a,const PkgInfo &b)
        {
            auto an = nameToFull(a.name);
            auto bn = nameToFull(b.name);
            return order == Qt::AscendingOrder ? an.localeAwareCompare(bn) < 0
                                            : an.localeAwareCompare(bn) > 0;;});
        break;

        //sort by types
    case Columns::TYPE:
        qSort(downList.begin(),downList.end(),[&](const PkgInfo &a,const PkgInfo &b)
        { return order == Qt::AscendingOrder ? a.type < b.type
                                            : a.type > b.type; });
        Types lastType;
        //TODO: maybe replace lasttype
        for(int i = 0,first=0;i < downList.size();i++)
        {
            if (!i)
                lastType = downList[0].type;
            else {
                if(lastType != downList[i].type || i == downList.size()-1){
                    qSort(downList.begin()+first,downList.begin()+i-1,[&](const PkgInfo &a,const PkgInfo &b) {
                        auto an = nameToFull(a.name);
                        auto bn = nameToFull(b.name);
                        return an.localeAwareCompare(bn) < 0;});
                    first=i;
                }
                lastType = downList[i].type;
            }
        }

        break;

        //sort by size
    case Columns::SIZE:
        qSort(downList.begin(),downList.end(),[&](const PkgInfo &a,const PkgInfo &b)
        {return order == Qt::AscendingOrder ? a.size < b.size
                                            : a.size > b.size;});
        break;

        //sort by state
    case Columns::STATE:
        qSort(downList.begin(),downList.end(),[&](const PkgInfo &a, const PkgInfo &b)
        {return order == Qt::AscendingOrder ? a.state < b.state
                                            :a.state > b.state;});
        break;
    }
    emit dataChanged(createIndex(0,0),createIndex(rowCount()-1,columnCount()-1),
                     QVector<int>() << Qt::EditRole);
    emit sorted();
}

int DownloadModel::find(const QString &name) const
{
    for(int i=0;i<downList.size();i++)
        if(downList[i].name==name)
            return i;
    return -1;
}

int DownloadModel::count() const
{
    return downList.size();
}

int DownloadModel::countLangPairsInstalled() const
{
    int cnt = 0;
    for(int i=0;i<downList.size();++i)
        if(downList[i].type == Types::LANGPAIRS && downList[i].state == States::UNINSTALL)
            cnt++;
    return cnt;
}
