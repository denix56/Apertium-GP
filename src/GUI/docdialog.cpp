#include "docdialog.h"
#include "ui_filedialog.h"
const QStringList DocDialog::fileTypes
{
    "txt", "docx", "pptx", "html",
    "rtf", "odt", "xlsx", "xtg"
};

DocDialog::DocDialog(GpMainWindow *parent)
    : FileDialog(parent)
{
    setWindowTitle(tr("Document Translation"));
    ui->browseBtn->setText(ui->browseBtn->text().replace(tr("file"), tr("document")));
    ui->label_3->setText(ui->label_3->text().replace(tr("file"), tr("document")));

    auto translator = parent->getTranslator();
    connect(this, &DocDialog::fileForTransChoosed, translator, &Translator::fileTranslate);
    connect(ui->DropWidget, &DragnDropWidget::fileDropped, this, &DocDialog::fileForTransChoosed);
    connect(translator, &Translator::fileTranslated, this, &DocDialog::showPostFileTransDlg);
    connect(translator, &Translator::fileTranslateRejected, this, &DocDialog::fileTranslateFailed);
}

DocDialog::~DocDialog()
{
    delete ui;
}

QStringList DocDialog::getFileTypes() const
{
    return fileTypes;
}

