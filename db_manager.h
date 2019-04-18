#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <QObject>
#include <QString>
#include <QSqlDatabase>
#include <QVector>

enum class DB_Event
{
    DB_CONNECTION_FAILURE
};

class DB_Manager : public QObject
{
    Q_OBJECT

public:
    explicit DB_Manager( QObject *parent, const QString &dbName );
    virtual ~DB_Manager();

    bool isOk() const;

    // queries
    QVector<QString> getLangs() const;
    int getLangId( const QString &langTag ) const;
    int getWordId( const QString &word, const int &lang_id ) const;

    bool isTranslatedWord( const QString &word, const int &lang_id ) const;
    bool isKnownWord( const QString &word, const int &lang_id ) const;
    QString getCurrentNativeLang() const;
    void translate( const QString &nativeWord, const int &nativeLangId,
                    const QString &foreignWord, const int &foreignLangId ) const;

private:
    bool tableExists( const QString &tableName ) const;
    void insertNewWord( const QString &word, const int &lang_id ) const;

    QSqlDatabase db;
    QString dbName;
};

#endif // DB_MANAGER_H
