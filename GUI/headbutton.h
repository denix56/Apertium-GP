#ifndef HEADBUTTON_H
#define HEADBUTTON_H
#include <QPushButton>

class HeadButton : public QPushButton
{
    Q_OBJECT
public:
    HeadButton(QWidget* parent=0);
    ~HeadButton();

public slots:
    void changeButtonColor(bool);

private slots:
    void denySameButtonClick();
    void denySameButtonClick2();


private:
    bool wasClicked;
    bool once;
    void paintEvent(QPaintEvent *);
    int lastFontWidth;
};

#endif // HEADBUTTON_H
