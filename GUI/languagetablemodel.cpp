#include "languagetablemodel.h"
#include <QStringList>
#include <QDebug>
languageTableModel::languageTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    for(int i=0;i<rowN;i++)
        list << "";
}

languageTableModel::languageTableModel(QStringList &list, QObject *parent)
    : QAbstractTableModel(parent)
{
    this->list = list;
}

int languageTableModel::rowCount(const QModelIndex &parent) const
{
    return rowN;
}

int languageTableModel::columnCount(const QModelIndex &parent) const
{
    if (list.size()==0)
        return 1;
    return list.size()/rowN + (list.size() % rowN == 0 ? 0 : 1);
}

QVariant languageTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role==Qt::DisplayRole)
    {
        auto ans = list.at((index.column())*rowN+index.row());
        return QVariant(ans);
    }
    return QVariant();
}

bool languageTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int pos = index.column()*rowN+index.row();
    //if not in the end
    if (pos <list.size())
        list.replace(pos,value.toString());
    else
    {
        if (index.row()==0)
            beginInsertColumns(QModelIndex(),columnCount(),columnCount());
        list << value.toString();
        for (int i = index.row()+1; i<rowCount();i++)
            list << "";
        if (index.row()==0)
            endInsertColumns();
    }
    //Alphabethic sorting
    qSort(list.begin(),list.begin()+itemCount());
    emit dataChanged(createIndex(0,0), createIndex(rowCount()-1,columnCount()-1), QVector<int>() << Qt::EditRole);
    return true;
}
int languageTableModel::currentColumnRowCount(const QModelIndex &index)
{
    if (columnCount()-1 > index.column())
        return rowN;
    else    
        for (int i = 0;i<rowCount();++i)
            if (list.at(rowN*index.column() + i)=="")
                return i;
}

bool languageTableModel::addItem(const QVariant &value)
{
    int row = currentColumnRowCount(createIndex(0,columnCount()-1));
    if (row==rowCount())
        row = 0;

    auto n = list.size();
    if (list.at(columnCount()*rowN-1)!="")
        ++n;
    return setData(createIndex(row, n/rowN + (n % rowN == 0 ? 0 : 1)-1),value);
}

bool languageTableModel::removeItem(const int &row, const int &column)
{
        list.removeAt(column*rowN + row);
        list << "";
        if (!currentColumnRowCount(createIndex(0,columnCount()-1)) && columnCount()!=1)
        {
            beginRemoveColumns(QModelIndex(),columnCount()-1,columnCount()-1);
            list.erase(list.begin()+(columnCount()-1)*rowN, list.begin()+columnCount()*rowN);
            endRemoveColumns();
        }
    //Alphabethic sorting
    qSort(list.begin(),list.begin()+itemCount());
    emit dataChanged(createIndex(0,0), createIndex(rowCount()-1,columnCount()-1), QVector<int>() << Qt::EditRole);
    return true;
}

Qt::ItemFlags languageTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled;
}

bool languageTableModel::setNumberOfRows(int &n)
{
    if (rowN<=0)
    {
        qDebug() << "Invalid number of rows";
        return false;
    }
    rowN = n;
    return true;
}

void languageTableModel::clear()
{
    beginResetModel();
    list.clear();
    for(int i=0;i<rowN;i++)
        list << "";
    endResetModel();
    //emit dataChanged(createIndex(0,0),createIndex(rowCount()-1, columnCount()-1), QVector<int>() << Qt::EditRole);
}

QModelIndex languageTableModel::findText(QString value) const
{
    int pos = list.indexOf(value);
    if (pos != -1)
        return createIndex(pos % rowN, pos / rowN);
    else
        return createIndex(-1,-1);
}

int languageTableModel::itemCount() const
{
    int count = 0;
    for (int i = 0; i < list.size(); i++)
    {
        if (list.at(i) == "")
            break;
        count++;
    }
    return count;
}

int languageTableModel::maxRowCount() const
{
    return rowN;
}
