#pragma once

#include "filedialog.h"


class DocDialog : public FileDialog
{
    Q_OBJECT
public:
    DocDialog(GpMainWindow *parent);

    virtual ~DocDialog();

    virtual QStringList getFileTypes() const;

    static const QStringList fileTypes;
};
