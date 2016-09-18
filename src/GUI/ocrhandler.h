#pragma once

#include <QObject>
#include <memory>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

class OcrHandler : public QObject
{
    Q_OBJECT
public:
    explicit OcrHandler(QObject *parent);

    int init(QString lang);

    ~OcrHandler();

signals:
    void recognized(QString path);
    //void failedToStart();

public slots:
    void recognize(const QString &path);

private:
    std::unique_ptr<tesseract::TessBaseAPI> api;
};
