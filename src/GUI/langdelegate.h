<<<<<<< HEAD
#ifndef DELEGATE_H
#define DELEGATE_H
#include <QItemDelegate>
class LangDelegate : public QItemDelegate
{
public:
    LangDelegate(QObject *parent=0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawFocus(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect) const;
};

#endif // DELEGATE_H
=======
#ifndef DELEGATE_H
#define DELEGATE_H
#include <QItemDelegate>
class LangDelegate : public QItemDelegate
{
public:
    LangDelegate(QObject *parent=0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawFocus(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect) const;
};

#endif // DELEGATE_H
>>>>>>> a80b0491c6c41ea9ecc8150152ed6c393b815b83
