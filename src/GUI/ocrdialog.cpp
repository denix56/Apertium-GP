#include <QMessageBox>
#include "ocrdialog.h"
#include "ui_filedialog.h"
const QStringList OcrDialog::fileTypes {
    "jpg", "png", "tiff", "bmp",
    "pnm", "gif", "ps", "pdf", "webp"
};

OcrDialog::OcrDialog(GpMainWindow *parent) :
    FileDialog(parent)
{  
//TODO: automatic restart
    handler = new OcrHandler(this);
    if (handler->init(parent->getCurrentSourceLang())) {
            if(QMessageBox::critical(
                        this, tr("Tesseract OCR error"),
                        tr("This Tesseract language package is not installed. Install ")
                        + "tesseract-" + parent->getCurrentSourceLang()
                        +tr(" via package installer and retry. "
                            "Do you want to proceed to package installation?"),QMessageBox::Ok, QMessageBox::Cancel)
                    == QMessageBox::Ok)
                emit parent->ocrFailed();
            close();
    }

    setWindowTitle(tr("OCR Image Translation"));
    ui->browseBtn->setText(ui->browseBtn->text().replace(tr("file"), tr("image")));
    ui->label_3->setText(ui->label_3->text().replace(tr("file"), tr("image")));

    connect(ui->DropWidget, &DragnDropWidget::fileDropped, this, &OcrDialog::fileForTransChoosed);
    connect(this, &OcrDialog::fileForTransChoosed, handler, &OcrHandler::recognize);

    connect(handler, &OcrHandler::recognized, this, &OcrDialog::ocrFinished);

    connect(this, &OcrDialog::ocrFinished, [&](){
        if (fileTransWaitDlg != nullptr) fileTransWaitDlg->accept();
        this->close();});
}

OcrDialog::~OcrDialog()
{
    delete ui;
}

QStringList OcrDialog::getFileTypes() const
{
    return fileTypes;
}
