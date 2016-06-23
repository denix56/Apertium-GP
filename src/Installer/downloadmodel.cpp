#include "downloadmodel.h"
#include "initializer.h"
#include <QDebug>
#include <QSize>
DownloadModel::DownloadModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    stateNames.insert(INSTALL,tr("Install"));
    stateNames.insert(UPDATE,tr("Update"));
    stateNames.insert(UNINSTALL,tr("Uninstall"));
    stateNames.insert(DOWNLOADING,tr("Cancel"));
    stateNames.insert(UNPACKING,tr("Unpacking"));

    typeNames.insert(TOOLS, tr("Tools"));
    typeNames.insert(LANGPAIRS, tr("Langpairs"));
}



QVariant DownloadModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation==Qt::Horizontal)
            if (role == Qt::DisplayRole)
            {
                auto c = static_cast<columns>(section);
                switch(c)
                {
                case NAME:
                    return QVariant(tr("Name"));
                case TYPE:
                    return QVariant(tr("Type"));
                case SIZE:
                    return QVariant(tr("Size"));
                case STATE:
                    return QVariant(tr("Action"));
                default:
                    return QVariant();
                }
            }
            else
                return QVariant();
}

int DownloadModel::rowCount(const QModelIndex &parent) const
{
    return downList.size();
}

int DownloadModel::columnCount(const QModelIndex &parent) const
{
    return 4;
}

QVariant DownloadModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role == Qt::DisplayRole)
    {
        int i = index.row();
        switch(index.column())
        {
        case 0:
            if (downList[i].type == LANGPAIRS)
                return QVariant(nameToFull(downList[i].name));
            else
                return QVariant(downList[i].name);
        case 1:
            return QVariant(typeNames[downList[i].type]);
        case 2:
            return QVariant(formatBytes(downList[i].size));
        case 3:
            return QVariant(stateNames[downList[i].state]);
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
        auto c = static_cast<columns>(index.column());
        switch(c)
        {
        case NAME:
            downList[i].name = value.toString();
            break;
        case TYPE:
            downList[i].type = static_cast<types>(value.toUInt());
            break;
        case SIZE:
            if (downList[i].state==DOWNLOADING)
                downList[i].progress = value.toUInt();
            else
                downList[i].size = value.toUInt();
            break;
        case STATE:
            downList[i].state = static_cast<states>(value.toUInt());
            break;
        }
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

bool DownloadModel::addItem(const file &f)
{
    beginInsertRows(QModelIndex(),rowCount(),rowCount());
    downList.push_back(f);
    endInsertRows();
    emit dataChanged(createIndex(rowCount()-1,0),createIndex(rowCount()-1,columnCount()-1),
                     QVector<int>() << Qt::EditRole);
    return true;
}

const file *DownloadModel::item(const int &row) const
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
    auto c = static_cast<columns>(column);
    switch (column)
    {
    //sort by names
    case NAME:
        qSort(downList.begin(),downList.end(),[&order](const file &a,const file &b)
        {
            auto an = nameToFull(a.name);
            auto bn = nameToFull(b.name);
            return order == Qt::AscendingOrder ? an.localeAwareCompare(bn) < 0
                                            : an.localeAwareCompare(bn) > 0;;});
        break;

        //sort by types
    case TYPE:
        qSort(downList.begin(),downList.end(),[&order](const file &a,const file &b)
        {return order == Qt::AscendingOrder ? a.type < b.type
                                            : a.type > b.type;});
        types lastType;
        //TODO: maybe replace lasttype
        for(int i = 0,first=0;i < downList.size();i++)
        {
            if (!i)
                lastType = downList[0].type;
            else
            {
                if(lastType!=downList[i].type || i==downList.size()-1){
                    qSort(downList.begin()+first,downList.begin()+i-1,[](const file &a,const file &b)
                    {
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
    case SIZE:
        qSort(downList.begin(),downList.end(),[&order](const file &a,const file &b)
        {return order == Qt::AscendingOrder ? a.size < b.size
                                            : a.size > b.size;});
        break;

        //sort by state
    case STATE:
        qSort(downList.begin(),downList.end(),[&order](const file &a, const file &b)
        {return order == Qt::AscendingOrder ? a.state < b.state
                                            :a.state > b.state;});
        break;
    }
    emit dataChanged(createIndex(0,0),createIndex(rowCount()-1,columnCount()-1),
                     QVector<int>() << Qt::EditRole);
    emit sorted();
}

int DownloadModel::find(const QString &name)
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
        if(downList[i].type==LANGPAIRS && downList[i].state==UNINSTALL)
            cnt++;
    return cnt;
}
