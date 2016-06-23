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
