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
    int currentColumnRowCount(const QModelIndex &index);
    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool setNumberOfRows(int &n);

    void clear();

    int itemCount() const;
    int maxRowCount() const;
    QModelIndex findText(QString value);

private:
    int rowN = 8;
    QStringList list;
};

#endif // LANGUAGETABLEMODEL_H
