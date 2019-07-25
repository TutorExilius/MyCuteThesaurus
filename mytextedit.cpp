#include "mytextedit.h"

#include "mainwindow.h"

#include <QDebug>
#include <QAction>
#include <QTextEdit>
#include <QMoveEvent>
#include <QScrollBar>
#include <QCoreApplication>


MyTextEdit::MyTextEdit( QWidget *parent )
: QTextEdit( parent )
{
    QAction* escAction = new QAction( "text4ESC", this );
    escAction->setShortcut( Qt::Key_Escape );
    escAction->setShortcutContext( Qt::WindowShortcut );

    QObject::connect( escAction, &QAction::triggered,
                      this, &MyTextEdit::onEscapeTriggered,
                      Qt::UniqueConnection );
}

bool MyTextEdit::isPartOfWordSeperators( const QChar &ch ) const
{
    return this->part_of_word_sepearators.contains( ch );
}

int MyTextEdit::getScrollPosition() const
{
    return this->verticalScrollBar()->value();
}

void MyTextEdit::setScrollPosition( const int position )
{
    this->verticalScrollBar()->setValue( position );
}

void MyTextEdit::keyPressEvent( QKeyEvent *event )
{
    QTextEdit::keyPressEvent( event );

    switch( event->key() )
    {
    case Qt::Key_Control:
        this->setReadOnly( true );
        break;
    }
}

void MyTextEdit::keyReleaseEvent( QKeyEvent *event )
{
    QTextEdit::keyPressEvent( event );

    switch( event->key() )
    {
    case Qt::Key_Control:
        this->setReadOnly( false );
        break;
    }
}

void MyTextEdit::mouseDoubleClickEvent( QMouseEvent *e )
{
    QTextEdit::mouseDoubleClickEvent( e );

    int pos = this->textCursor().positionInBlock();

    qDebug() << "Clicked pos: " << pos;

    if( pos != 0 )
        --pos;

    const QString text = this->toPlainText();

    if( text.isEmpty() )
    {
        return;
    }

    int startPos = pos;
    int endPos = pos;

    // find start
    for( int i = startPos; i>=0; i-- )
    {
        //if( !text[i].isLetter() && !this->part_of_word_sepearators.contains( text[i] ) )
        if( text[i].isSpace() )
        {
            startPos = i+1;
            break;
        }

        if( i == 0 )
        {
            startPos = 0;
        }
    }


    // find end
    endPos = startPos;
    for( int i = endPos; i<text.size(); i++ )
    {
        if( !text[i].isLetter() && !this->part_of_word_sepearators.contains( text[i] ) )
        //if( text[i].isSpace() )
        {
            endPos = i-1;
            break;
        }

        if( i == text.size()-1 )
        {
            endPos =  text.size() - 1;
        }
    }

    QTextCursor c = this->textCursor();
    c.setPosition( startPos );
    c.setPosition( endPos + 1, QTextCursor::KeepAnchor );
    this->setTextCursor( c );

    emit doubleClicked();
}

void MyTextEdit::onEscapeTriggered()
{
    emit escapeTriggered();
}
