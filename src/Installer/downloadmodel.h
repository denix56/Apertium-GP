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

enum states {INSTALL, UPDATE, UNINSTALL, DOWNLOADING, UNPACKING};
enum types {LANGPAIRS, TOOLS};
enum columns {NAME, TYPE, SIZE, STATE};

struct file
{
    QString name;
    QUrl link;
    uint size, progress;
    QString lm;
    states state;
    types type;
    bool highlight;
    file (QString name=QString(), types type=TOOLS, uint size=0, QUrl link=QUrl(),
          states state=INSTALL, QString lm=QString(), bool highlight = false, int progress=0)
    {
        this->name = name;
        this->type = type;
        this->size = size;
        this->link = link;
        this->state = state;
        this->lm = lm;
        this->progress = progress;
        this->highlight = highlight;
    }
};

inline QString formatBytes(double val) {
    const char *suf = "B";
    if (val >= 1024) {
        val /= 1024;
        suf = "KiB";
    }
    if (val >= 1024) {
        val /= 1024;
        suf = "MiB";
    }
    return QString("%1 %2").arg(val, 0, 'f', 2).arg(suf);
}

class DownloadModel : public QAbstractTableModel
{
    Q_OBJECT
signals:
    void sorted();
public:
    explicit DownloadModel(QObject *parent = 0);
    void reset();
    int find(const QString &name);
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

    bool addItem(const file &f);
    const file *item(const int &row) const;
    void sort(int column, Qt::SortOrder order);
private:
    QVector <file> downList;

    const QMap <states, QString> stateNames {
        {INSTALL,tr("Install")}, {UPDATE,tr("Update")},
        {UNINSTALL,tr("Uninstall")}, {DOWNLOADING,tr("Cancel")},
        {UNPACKING,tr("Unpacking")}
    };
    const QMap <types, QString> typeNames {
        {LANGPAIRS, tr("Langpairs")},
        {TOOLS, tr("Tools")}
    };
};

#endif // DOWNLOADMODEL_H