#ifndef MYTEXTEDIT_H
#define MYTEXTEDIT_H

#include <QTextEdit>
#include <QVector>

class MyTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    explicit MyTextEdit( QWidget *parent = nullptr );
    bool isPartOfWordSeperators( const QChar &ch ) const;

protected:
    void mouseDoubleClickEvent( QMouseEvent *e ) override;

signals:
    void doubleClicked();

public slots:

private:
    // TODO: initialise this vector from database!
    QVector<QChar> part_of_word_sepearators{ '-', L'´', '`', L'’', '\'' };   // EXAMPLE
};

#endif // MYTEXTEDIT_H
