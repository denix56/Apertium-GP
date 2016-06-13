#ifndef MANAGERHELPER_H
#define MANAGERHELPER_H
#include <QStringList>
#include <QObject>
class ManagerHelper : public QObject
{
    Q_OBJECT
public:
    ManagerHelper(QObject* parent=0);
     //find out current package manager
    void chooseManager();
    QString getManager() const;
    QString install(QStringList packages) const;
    QString remove(QStringList packages) const;
    QString update() const;
    QString search(QString package) const;
    unsigned long long getSize(const QString &package) const;
private:
    QString mngr;
};

#endif // MANAGERHELPER_H
