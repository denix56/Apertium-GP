#ifndef TRAYWIDGETACTION_H
#define TRAYWIDGETACTION_H

#include <QObject>
#include <QWidget>
#include <QWidgetAction>
#include <QComboBox>
#include <QSortFilterProxyModel>

class TrayWidgetAction : public QWidgetAction
{
    Q_OBJECT
public:
    TrayWidgetAction(QObject *parent = 0);

    inline QComboBox *const getInputComboBox() const
    {
        return inputLangs;
    }

    inline QComboBox *const getOutputComboBox() const
    {
        return outputLangs;
    }
protected:
    QWidget *createWidget(QWidget *parent);

private:
    QComboBox *inputLangs;
    QComboBox *outputLangs;
    QSortFilterProxyModel * proxy1;
};

#endif // TRAYWIDGETACTION_H
