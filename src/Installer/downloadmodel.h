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

#include "initializer.h"

enum class States {INSTALL, UPDATE, UNINSTALL, DOWNLOADING, UNPACKING};

enum class Types {LANGPAIRS, TOOLS};

enum class Columns {NAME, TYPE, SIZE, STATE};

Q_DECLARE_METATYPE(States)
Q_DECLARE_METATYPE(Types)
Q_DECLARE_METATYPE(Columns)

struct PkgInfo
{
    QString name;
    Types type;
    uint size;
    QUrl link;
    States state;
    QString lastModified;
    uint progress;
    bool highlight;

    PkgInfo (const QString name = QString(), Types type = Types::TOOLS, uint size = 0, QUrl link = QUrl(),
          States state = States::INSTALL, QString lastModified = QString(), bool highlight = false, int progress = 0)
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

    void reset();

    int find(const QString &name) const;

    int count() const;

    int countLangPairsInstalled() const;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool addItem(const PkgInfo &f);

    const PkgInfo *item(const int &row) const;

    void sort(int column, Qt::SortOrder order);

private:
    inline QString formatBytes(double val) const
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

    inline QString nameToFull(QString pair) const
    {
        pair.remove("apertium-");
        int i = pair.indexOf('-');
        QString sourceLang = pair.left(i);
        QString targetLang = pair.mid(i+1);
        if (Initializer::langNamesMap.contains(sourceLang))
            sourceLang = Initializer::langNamesMap[sourceLang];

        if (Initializer::langNamesMap.contains(targetLang))
            targetLang = Initializer::langNamesMap[targetLang];

        return sourceLang + " - " + targetLang;
    }

    QVector <PkgInfo> downList;

    const QMap <States, QString> stateNames {
        { States::INSTALL, tr("Install") }, { States::UPDATE,tr("Update") },
        { States::UNINSTALL,tr("Uninstall") }, { States::DOWNLOADING,tr("Cancel") },
        { States::UNPACKING,tr("Unpacking") }
    };

    const QMap <Types, QString> typeNames {
        { Types::LANGPAIRS, tr("Langpairs") },
        { Types::TOOLS, tr("Tools") }
    };
};

#endif // DOWNLOADMODEL_H
