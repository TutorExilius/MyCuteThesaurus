#ifndef WORD_H
#define WORD_H

#include <QString>
#include <QVector>

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
    bool hasTranslations() const;
    QVector<QString> getTranslations() const;
    void setTranslations( const QVector<QString> &translations );
    void addTranslation( const QString &translation );

private:
    QString content;
    QVector<QString> translations;
    TYPE type;
};

#endif // WORD_H
