#ifndef MYTEXTEDIT_H
#define MYTEXTEDIT_H

#include <QTextEdit>

class MyTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    explicit MyTextEdit( QWidget *parent = nullptr );


protected:
    void mouseDoubleClickEvent( QMouseEvent *e ) override;

signals:
    void doubleClicked();

public slots:

};

#endif // MYTEXTEDIT_H
