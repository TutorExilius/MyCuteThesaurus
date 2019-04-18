#include "mytextbox.h"
#include "ui_mytextbox.h"

MyTextBox::MyTextBox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MyTextBox)
{
    ui->setupUi(this);
}

MyTextBox::~MyTextBox()
{
    delete ui;
}
