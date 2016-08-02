#include "traywidgetaction.h"
#include "trayinputextedit.h"
#include <QGridLayout>
#include <QStandardItemModel>
#include <QEventLoop>

TrayWidgetAction::TrayWidgetAction(QObject *parent)
    :QWidgetAction(parent)
{


}
QWidget *TrayWidgetAction::createWidget(QWidget *parent)
{
    auto trayWidget = new QWidget(parent);
    inputLangs = new QComboBox(trayWidget);
    outputLangs = new QComboBox(trayWidget);
    QStandardItemModel* model = new QStandardItemModel(inputLangs);
    proxy1 = new QSortFilterProxyModel();
    proxy1->setSourceModel(model);
    proxy1->sort(0);
    inputLangs->setModel(proxy1);

    auto input = new TrayInpuTextEdit(trayWidget);
    auto output = new QTextEdit(trayWidget);
    auto grid = new QGridLayout(trayWidget);

    input->setFixedSize(110,70);
    output->setFixedSize(110,70);

    grid->addWidget(input,0,0,1,1,Qt::AlignCenter);
    grid->addWidget(output,0,1,1,1,Qt::AlignCenter);
    grid->addWidget(inputLangs,1,0,1,1);
    grid->addWidget(outputLangs,1,1,1,1);
    grid->setContentsMargins(1,1,1,1);

    trayWidget->setFixedSize(225,100);
    trayWidget->setLayout(grid);
    return trayWidget;
}
