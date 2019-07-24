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
    int getScrollPosition() const;
    void setScrollPosition( const int position );

protected:
    void mouseDoubleClickEvent( QMouseEvent *e ) override;

signals:
    void doubleClicked();
    void escapeTriggered();

public slots:
    void onEscapeTriggered();

private:
    // TODO: initialise this vector from database!
    QVector<QChar> part_of_word_sepearators{ '-', L'´', '`', L'’', '\'' };   // EXAMPLE
};

#endif // MYTEXTEDIT_H
