#ifndef INPUTTEXTEDIT_H
#define INPUTTEXTEDIT_H
#include <QTextEdit>
#include <QTimer>

class InputTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit InputTextEdit(QWidget* parent = 0);

signals:
    void printEnded();
private:
    QTimer timer;

};

#endif // INPUTTEXTEDIT_H
