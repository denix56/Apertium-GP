#ifndef DOCTRANSLATE_H
#define DOCTRANSLATE_H
#include "apertiumgui.h"
#include "dragndropwidget.h"
#include <QWidget>
namespace Ui {
class DocTranslate;
}

class DocTranslate : public QWidget
{
    Q_OBJECT

public:
    explicit DocTranslate(ApertiumGui *parent = 0);
    ~DocTranslate();
    static const QStringList fileTypes;
signals:
    void docForTransChoosed(QString filePath);
private slots:
    void on_browseBtn_clicked();
    void showPostDocTransDlg(QString trFilePath);

private:
    Ui::DocTranslate *ui;   
    ApertiumGui *parent;
};

#endif // DOCTRANSLATE_H
