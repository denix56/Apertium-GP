#ifndef TABLECOMBOBOX_H
#define TABLECOMBOBOX_H
#include "languagetablemodel.h"
#include <QObject>
#include <QComboBox>
#include <QTableView>

class TableComboBox : public QComboBox
{
    Q_OBJECT
public:
    TableComboBox(QWidget *parent=nullptr);
    languageTableModel *model() const;
    QTableView *view() const;
    void setView(QTableView *view);
    void setModel(languageTableModel *view);

protected:
    void wheelEvent(QWheelEvent *e);

private slots:
    void resizeT();
private:
    const int COLWIDTH = 100;
};

#endif // TABLECOMBOBOX_H
