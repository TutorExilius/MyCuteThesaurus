#ifndef WORD_H
#define WORD_H

#include <QString>

enum class TYPE
{
    WORD,
    LINK,
    UNKOWN
};

class Word
{
public:
    Word();
    explicit Word( const QString &word, const TYPE type );

    QString getContent() const;

    bool isWordType() const;

private:
    QString content;
    TYPE type;
};

#endif // WORD_H
