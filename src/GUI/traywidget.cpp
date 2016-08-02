#include "traywidget.h"
#include "ui_traywidget.h"
#include <QDesktopWidget>
#include <QDebug>

TrayWidget::TrayWidget(QWidget *parent) :
    QWidget(parent, Qt::Dialog | Qt::FramelessWindowHint),
    ui(new Ui::TrayWidget)
{
    ui->setupUi(this);
    auto desktop = qApp->desktop();
    setGeometry(desktop->availableGeometry().width()-this->width(),
                      desktop->availableGeometry().height()-this->height(),
                      this->width(),this->height());
    connect(ui->textEdit,&TrayInputTextEdit::printEnded,this,&TrayWidget::prindEnded);
}

void TrayWidget::translationReceived(const QString &result)
{
    ui->textEdit_2->clear();
    auto cursor = ui->textEdit_2->textCursor();
    cursor.movePosition(QTextCursor::Start);
    auto format = cursor.charFormat();
    format.setForeground(Qt::black);
    cursor.insertText(result, format);
    cursor.movePosition(QTextCursor::Start);
    while(!cursor.atEnd()) {
        auto cursor1 = cursor.document()->
                find(QRegularExpression ("[\\*#]\\w+\\W?"),cursor.position());
        if (cursor1.isNull())
            break;
        cursor = cursor1;
        auto format = cursor.charFormat();
        format.setForeground(Qt::red);
        cursor.insertText(cursor.selectedText().mid(1),format);
    }
    ui->textEdit_2->setTextCursor(cursor);
}

QComboBox *const TrayWidget::inputComboBox() const
{
    return ui->comboBox;
}

QComboBox *const TrayWidget::outputComboBox() const
{
    return ui->comboBox_2;
}
TrayWidget::~TrayWidget()
{
    delete ui;
}
