#include "languagetablemodel.h"
#include <QStringList>
#include <QDebug>
languageTableModel::languageTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

languageTableModel::languageTableModel(QStringList &list, QObject *parent)
    : QAbstractTableModel(parent)
{
    this->list = list;
}

int languageTableModel::rowCount(const QModelIndex &parent) const
{
    return itemCount()<rowN ? list.size() : rowN;
}

int languageTableModel::columnCount(const QModelIndex &parent) const
{
    if (list.size()==0)
        return 1;
    return list.size()/rowN + (list.size() % rowN == 0 ? 0 : 1);
}

QVariant languageTableModel::data(const QModelIndex &index, int role) const
{
    int pos = index.column()*rowN+index.row();
    if (!index.isValid() || pos>=list.size())
        return QVariant();
    if (role==Qt::DisplayRole)
    {
        auto ans = list.at(pos);
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
        if(list.size()>=rowN) {
        if (index.row()==0)
            beginInsertColumns(QModelIndex(),columnCount(),columnCount());
        list << value.toString();
        for (int i = index.row()+1; i<rowCount();i++)
            list << "";
        if (index.row()==0)
            endInsertColumns();
        }
        else {
            beginInsertRows(QModelIndex(),rowCount(),rowCount());
            list << value.toString();
            endInsertRows();
        }
   }
    //Alphabethic sorting
    qSort(list.begin(),list.begin()+itemCount());
    emit dataChanged(createIndex(0,0), createIndex(rowCount()-1,columnCount()-1), QVector<int>() << Qt::EditRole);
    return true;
}
int languageTableModel::currentColumnRowCount(const QModelIndex &index)
{
    if (columnCount() > index.column() + 1)
        return rowN;
    else    
        for (int i = 0;i<rowCount();++i)
            if (list.at(rowN*index.column() + i)=="")
                return i;
    return 0;
}

bool languageTableModel::addItem(const QVariant &value)
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

bool languageTableModel::removeItem(const int &row, const int &column)
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
    endResetModel();
    emit dataChanged(createIndex(0,0),createIndex(rowCount()-1, columnCount()-1), QVector<int>() << Qt::EditRole);
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
            return count;
        count++;
    }
    return count;
}

int languageTableModel::maxRowCount() const
{
    return rowN;
}
