#include <QApplication>
#include <QDebug>
#include "ocrhandler.h"
#include "initializer.h"

OcrHandler::OcrHandler(QObject *parent) :
    QObject(parent),
    api(new tesseract::TessBaseAPI())
{}
/*!
 * To install tessdata on Windows, download and install it manually
 * in the Program specified tessdata path (AppData_dir/apertium-gp/tessdata
 * for default)
 */
int OcrHandler::init(QString lang)
{
    char *path = Initializer::conf->value("extra/ocr/tessdata_path", DATALOCATION + "/tessdata")
            .toString().toUtf8().data();
#ifdef Q_OS_WIN
    QDir().mkpath(path);
#else
    path = NULL;
#endif
    return api->Init(path, lang.toUtf8().data());
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
