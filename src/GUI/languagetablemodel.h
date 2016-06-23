<<<<<<< HEAD
#ifndef LANGUAGETABLEMODEL_H
#define LANGUAGETABLEMODEL_H

#include <QAbstractTableModel>

class languageTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit languageTableModel(QObject *parent = 0);
    explicit languageTableModel(QStringList &list, QObject *parent = 0);

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

#endif // LANGUAGETABLEMODEL_H
=======
#ifndef LANGUAGETABLEMODEL_H
#define LANGUAGETABLEMODEL_H

#include <QAbstractTableModel>

class languageTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit languageTableModel(QObject *parent = 0);
    explicit languageTableModel(QStringList &list, QObject *parent = 0);

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

#endif // LANGUAGETABLEMODEL_H
>>>>>>> a80b0491c6c41ea9ecc8150152ed6c393b815b83
