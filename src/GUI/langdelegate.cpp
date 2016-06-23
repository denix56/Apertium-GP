<<<<<<< HEAD
#include "langdelegate.h"
#include <QPainter>
LangDelegate::LangDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}
void LangDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (option.state & QStyle::State_HasFocus && !index.data().toString().isEmpty())
        painter->fillRect(option.rect,QColor(220,220,220));

    QItemDelegate::paint(painter,option,index);
}

void LangDelegate::drawFocus(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect) const
{
}
=======
#include "langdelegate.h"
#include <QPainter>
LangDelegate::LangDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}
void LangDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (option.state & QStyle::State_HasFocus && !index.data().toString().isEmpty())
        painter->fillRect(option.rect,QColor(220,220,220));

    QItemDelegate::paint(painter,option,index);
}

void LangDelegate::drawFocus(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect) const
{
}
>>>>>>> a80b0491c6c41ea9ecc8150152ed6c393b815b83
