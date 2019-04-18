#ifndef MYTEXTBOX_H
#define MYTEXTBOX_H

#include <QWidget>

namespace Ui {
class MyTextBox;
}

class MyTextBox : public QWidget
{
    Q_OBJECT

public:
    explicit MyTextBox(QWidget *parent = nullptr);
    ~MyTextBox();

private:
    Ui::MyTextBox *ui;
};

#endif // MYTEXTBOX_H
