#ifndef TRAYWIDGET_H
#define TRAYWIDGET_H

#include <QWidget>
#include <QComboBox>

namespace Ui {
class TrayWidget;
}

class TrayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TrayWidget(QWidget *parent = 0);

    QComboBox *const inputComboBox() const;

    QComboBox *const outputComboBox() const;

    ~TrayWidget();

signals:
    void prindEnded(QString text);

public slots:
    void translationReceived(const QString &result);

private:
    Ui::TrayWidget *ui;
};

#endif // TRAYWIDGET_H
