#include "word.h"

Word::Word()
: Word{ "", TYPE::UNKOWN }
{
}

Word::Word( const QString &content, const TYPE type )
: content{ content }
, type{ type }
{
}

QString Word::getContent() const
{
    return this->content;
}

bool Word::isWordType() const
{
    return ( this->type == TYPE::WORD );
}

bool Word::hasTranslations() const
{
    return !this->translations.isEmpty();
}

QVector<QString> Word::getTranslations() const
{
    return this->translations;
}

void Word::setTranslations( const QVector<QString> &translations )
{
    this->translations = translations;
}

void Word::setContent( const QString &content )
{
    this->content = content;
}

void Word::addTranslation( const QString &translation )
{
    if( !this->translations.contains( translation ) )
    {
        this->translations.push_back( translation );
    }
}

void Word::removeTranslation( const QString &translation )
{
    if( this->translations.contains( translation ) )
    {
        this->translations.removeAll( translation );
    }
}
