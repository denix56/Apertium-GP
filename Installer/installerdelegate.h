#ifndef INSTALLERDELEGATE_H
#define INSTALLERDELEGATE_H
#include <QStyledItemDelegate>

class InstallerDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    InstallerDelegate(QObject *parent=0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
signals:
    void stateChanged(int);
};

#endif // INSTALLERDELEGATE_H
