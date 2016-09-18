#include <QApplication>
#include <QDebug>
#include "ocrhandler.h"

OcrHandler::OcrHandler(QObject *parent) :
    QObject(parent),
    api(new tesseract::TessBaseAPI())
{}

int OcrHandler::init(QString lang)
{
    return api->Init(NULL, lang.toUtf8().data());
}

void OcrHandler::recognize(const QString &path)
{
    // Open input image with leptonica library
    Pix *image = pixRead(path.toUtf8().data());
    api->SetImage(image);
    // Get OCR result
    auto tmp = api->GetUTF8Text();
    QString result(tmp);
    delete[] tmp;
    pixDestroy(&image);
    emit recognized(result);
}

OcrHandler::~OcrHandler()
{
    api->End();
}
