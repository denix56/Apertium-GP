/*
* Copyright (C) 2016, Denys Senkin <denisx9.0c@gmail.com>
*
* This file is part of apertium-gp
*
* apertium-gp is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* apertium-gp is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with apertium-gp.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DOWNLOADMODEL_H
#define DOWNLOADMODEL_H
#include <QAbstractTableModel>
#include <QMap>
#include <QUrl>
#include <QObject>
#include <QDebug>

#include "initializer.h"

enum class State {INSTALL, UPDATE, UNINSTALL, DOWNLOADING, UNPACKING};

enum class Type {OCR, LANGPAIRS, TOOLS};

enum class Column {NAME, TYPE, SIZE, STATE};

Q_DECLARE_METATYPE(State)
Q_DECLARE_METATYPE(Type)
Q_DECLARE_METATYPE(Column)

struct PkgInfo {
    QString name;
    Type type;
    uint size;
    QUrl link;
    State state;
    QString lastModified;
    uint progress;
    bool highlight;

    PkgInfo (const QString name = QString(), Type type = Type::TOOLS, uint size = 0, QUrl link = QUrl(),
             State state = State::INSTALL, QString lastModified = QString(), bool highlight = false,
             int progress = 0)
        : name(name), type(type), size(size), link(link), state(state),
          lastModified(lastModified),  progress(progress), highlight(highlight)
    {
    }
};

class DownloadModel : public QAbstractTableModel
{
    Q_OBJECT
signals:
    void sorted();
public:
    explicit DownloadModel(QObject *parent = 0);

    static inline QString formatBytes(double val)
    {
        QString suf = tr("B");
        if (val >= 1024) {
            val /= 1024;
            suf = tr("KiB");
        }
        if (val >= 1024) {
            val /= 1024;
            suf = tr("MiB");
        }
        return QString("%1 %2").arg(val, 0, 'f', 2).arg(suf);
    }

    void reset();

    int find(const QString &name) const;

    int count() const;

    int countLangPairsInstalled() const;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool addItem(const PkgInfo &f);

    const PkgInfo *item(const int &row) const;

    void sort(int column, Qt::SortOrder order);

private:
    inline QString nameToFull(QString name) const
    {
        QString result;
        if (name.contains("apertium")) {
            name.remove("apertium-");
            int i = name.indexOf('-');
            QString sourceLang = name.left(i);
            QString targetLang = name.mid(i + 1);
            sourceLang = Initializer::langNamesMap.value(sourceLang, sourceLang);
            targetLang = Initializer::langNamesMap.value(targetLang, targetLang);
            result = sourceLang + " - " + targetLang;
        } else if (name.contains("tesseract")) {
            name.remove("tesseract-ocr-");
            int i = name.indexOf('-');
            QString extra = i > 0 ? " - " + name.mid(i + 1) : "";
            name = name.left(i);
            result = "Tesseract (" + Initializer::langNamesMap.value(name, name)
                     + extra + ")";
        }
        return result;
    }

    QVector <PkgInfo> downList;

    const QMap <State, QString> stateNames {
        { State::INSTALL, tr("Install") }, { State::UPDATE, tr("Update") },
        { State::UNINSTALL, tr("Uninstall") }, { State::DOWNLOADING, tr("Cancel") },
        { State::UNPACKING, tr("Unpacking") }
    };

    const QMap <Type, QString> typeNames {
        { Type::LANGPAIRS, tr("Language pairs") },
        { Type::TOOLS, tr("Tools") },
        { Type::OCR, tr("OCR") },
    };
};

#endif // DOWNLOADMODEL_H
