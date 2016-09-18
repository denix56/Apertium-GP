#pragma once

#include "filedialog.h"
#include "ocrhandler.h"

class OcrDialog : public FileDialog
{
    Q_OBJECT

public:
    explicit OcrDialog(GpMainWindow *parent = 0);

    virtual ~OcrDialog();

    virtual QStringList getFileTypes() const;

    static const QStringList fileTypes;

signals:
void ocrFinished(QString text);
private:
    OcrHandler *handler;

};

