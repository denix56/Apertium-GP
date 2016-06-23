#include "tablecombobox.h"
#include "langdelegate.h"
#include <QHeaderView>
#include <QDebug>
TableComboBox::TableComboBox(QWidget *parent)
    : QComboBox(parent)
{
    setView(new QTableView(this));
    setModel(new languageTableModel(view()));
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
    connect(model(),&languageTableModel::dataChanged,
            this,&TableComboBox::resizeT);
}

QTableView *TableComboBox::view() const
{
    return qobject_cast<QTableView*> (QComboBox::view());
}

languageTableModel *TableComboBox::model() const
{
    return qobject_cast<languageTableModel*>(QComboBox::model());
}

void TableComboBox::setView(QTableView *view)
{
    QComboBox::setView(view);
}

void TableComboBox::setModel(languageTableModel* model)
{
    QComboBox::setModel(model);
}

void TableComboBox::resizeT()
{
    //TODO: bug with no elements in model list
    if(model()->itemCount()==0)
        return;
    int colc = model()->columnCount();/*model()->itemCount()/model()->maxRowCount() +
            (model()->itemCount() % model()->rowCount() ? 1 : 0);*/
    for (int i=0; i < colc;i++)
        view()->setColumnWidth(i, COLWIDTH);
<<<<<<< HEAD
    view()->setMinimumWidth(COLWIDTH*(colc));
=======
    view()->setMinimumWidth(COLWIDTH*colc);
>>>>>>> a80b0491c6c41ea9ecc8150152ed6c393b815b83
    view()->setMinimumHeight((model()->rowCount())*view()->rowHeight(0));
}

void TableComboBox::wheelEvent(QWheelEvent *e)
{}
