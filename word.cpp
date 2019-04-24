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
