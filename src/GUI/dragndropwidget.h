#ifndef DRAGNDROPWIDGET_H
#define DRAGNDROPWIDGET_H

#include <QObject>
#include <QWidget>

class DragnDropWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DragnDropWidget(QWidget *parent = 0);
protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void paintEvent(QPaintEvent *event);
signals:
    void documentDropped(QString path);
public slots:
private:
    QString fileType;
};

#endif // DRAGNDROPWIDGET_H
