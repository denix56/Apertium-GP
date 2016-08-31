#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QObject>
#include <QApplication>
#include <QSharedMemory>

class SingleApplication : public QApplication
{
    Q_OBJECT
public:
    SingleApplication(int &argc, char **argv);
    ~SingleApplication();

    bool lock();

private:
    QSharedMemory *_singular; // shared memory !! SINGLE ACCESS
};

#endif // SINGLEAPPLICATION_H


