#include "db_manager.h"

#include <QDebug>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>

#include "log.h"

DB_Manager::DB_Manager( QObject *parent, const QString &dbName )
: QObject{ parent }
, dbName{ dbName }
{
    this->db = QSqlDatabase::addDatabase( "QSQLITE" );
    //this->db.setHostName("test.domain.de");
    this->db.setDatabaseName( dbName );
    //this->db.setUserName("");
    //this->db.setPassword("");

    if( !this->db.open() )
    {
        ::logError( "Connection with database failed" );
    }
    else
    {
        qDebug() << "Database connected";
    }

    if( !this->isOk() )
    {
        ::logError( "Missing tables in database" );
    }
    else
    {
        qDebug() << "All tables found. DB is ok.";
    }
}

DB_Manager::~DB_Manager()
{
    if( this->db.open() )
    {
        this->db.close();
        qDebug() << "Database disconnected";
    }
}

/*
std::shared_ptr<DB_Model_FollowerCountHistory> DB_Manager::getLastFollowerCountHistory() const
{
    QSqlQuery query( this->db );

    query.prepare( "SELECT max(id),* FROM follower_count_history" );

    if( query.exec() )
    {
        if( query.first() && !query.isNull("created_at") && !query.isNull("created_at") )
        {
            const QString created_at = query.value( "created_at" ).toString();
            const int follower_count = query.value( "follower_count" ).toInt();

            return std::shared_ptr<DB_Model_FollowerCountHistory>{ new DB_Model_FollowerCountHistory{ created_at, follower_count } };
        }
        else
        {
            return nullptr;
        }
    }
    else
    {
        qDebug() << "SqLite error:" << query.lastError().text();
    }

    return nullptr;
}

QMultiMap<int,std::shared_ptr<DB_Model_UserName>> DB_Manager::getUserNames() const
{
    QMultiMap<int,std::shared_ptr<DB_Model_UserName>> userNames;

    QSqlQuery query( this->db );

    query.prepare( "SELECT * FROM twitch_user_name_history" );

    if( query.exec() )
    {
        while( query.next() )
        {
            userNames.insert( query.value( "user_id" ).toInt(),
                              std::shared_ptr<DB_Model_UserName>(
                                  new DB_Model_UserName{ query.value( "user_id" ).toInt(),
                                            query.value( "name" ).toString(),
                                            query.value( "created_at" ).toString() } )
                              );
        }
    }
    else
    {
        qDebug() << "SqLite error:" << query.lastError().text();
    }

    return userNames;
}

QMultiMap<int,std::shared_ptr<DB_Model_DisplayName>> DB_Manager::getDisplayNames() const
{
    QMultiMap<int,std::shared_ptr<DB_Model_DisplayName>> displayNames;

    QSqlQuery query( this->db );

    query.prepare( "SELECT * FROM twitch_user_displayname_history" );

    if( query.exec() )
    {
        while( query.next() )
        {
            displayNames.insert( query.value( "user_id" ).toInt(),
                                 std::shared_ptr<DB_Model_DisplayName>(
                                     new DB_Model_DisplayName{ query.value( "user_id" ).toInt(),
                                        query.value( "displayname" ).toString(),
                                        query.value( "created_at" ).toString() } )
                                 );
        }
    }
    else
    {
        qDebug() << "SqLite error:" << query.lastError().text();
    }

    return displayNames;
}

QString DB_Manager::getDisplayName(const int &user_id) const
{
    QSqlQuery query( this->db );

    query.prepare( "SELECT max(id),* FROM twitch_user_displayname_history WHERE user_id = "
                  + QString::number( user_id ) );

    if( query.exec() )
    {
        if( query.next() && !query.isNull( "display_name" ) )
        {
            return query.value( "display_name" ).toString();
        }
    }
    else
    {
        qDebug() << "SqLite error:" << query.lastError().text();
    }

    return "";
}

QString DB_Manager::getName( const int &user_id ) const
{
    QSqlQuery query( this->db );

    query.prepare( "SELECT max(id),* FROM twitch_user_name_history WHERE user_id = "
                  + QString::number( user_id ) );

    if( query.exec() )
    {
        if( query.next() && !query.isNull( "name" ) )
        {
            return query.value( "name" ).toString();
        }
    }
    else
    {
        qDebug() << "SqLite error:" << query.lastError().text();
    }

    return "";
}

QMultiMap<int,std::shared_ptr<DB_Model_FollowHistory>> DB_Manager::getFollowHistory( const HistoryFollowerFlag &historyFollowerFlag ) const
{
    QMultiMap<int,std::shared_ptr<DB_Model_FollowHistory>> followHistory;

    QSqlQuery query( this->db );

    QString whereString;

    switch( historyFollowerFlag )
    {
        case HistoryFollowerFlag::ALL_FOLLOWS:
            whereString = " WHERE is_follower == 1";
            break;

        case HistoryFollowerFlag::ALL_DEFOLLOWS:
            whereString = " WHERE is_follower == 0";
            break;
    }

    query.prepare( "SELECT * FROM follow_history" + whereString );

    if( query.exec() )
    {
        while( query.next() )
        {
            followHistory.insert( query.value( "user_id" ).toInt(),
                                  std::shared_ptr<DB_Model_FollowHistory>(
                                        new DB_Model_FollowHistory{ query.value( "user_id" ).toInt(),
                                            query.value( "is_follower" ).toInt(),
                                            query.value( "created_at" ).toString() }
                                      ) );
        }
    }
    else
    {
        qDebug() << "SqLite error:" << query.lastError().text();
    }

    return followHistory;
}

QMap<int,TwitchUser*> DB_Manager::getTwitchUsers() const
{
    QMap<int,TwitchUser*> twitchUser;

    QSqlQuery query( this->db );

    query.prepare( "SELECT * FROM twitch_user" );

    if( query.exec() )
    {
        while( query.next() )
        {
            const int user_id = query.value( "user_id" ).toInt();
            const QString name = this->getName( user_id );
            const QString displayname = this->getDisplayName( user_id );

            twitchUser.insert( user_id,
                       new TwitchUser{ this->parent(),
                                     query.value( "user_id" ).toInt(),
                                     name,
                                     displayname,
                                     query.value( "last_update" ).toString(),
                                     query.value( "created_at" ).toString(),
                                     query.value( "updated_at" ).toString(),
                                     query.value( "bio" ).toString(),
                                     query.value( "logo" ).toString(),
                                     query.value( "type" ).toString(),
                                     query.value( "notifications" ).toInt() } );
        }
    }
    else
    {
        qDebug() << "SqLite error:" << query.lastError().text();
    }

    return twitchUser;
}

std::shared_ptr<TwitchUser> DB_Manager::getTwitchUser(const int &user_id) const
{
    QSqlQuery query( this->db );

    query.prepare( "SELECT * FROM twitch_user WHERE user_id = " + QString::number( user_id ) );

    if( query.exec() )
    {
        if( query.next() )
        {
            const QString name = this->getName( user_id );
            const QString displayname = this->getDisplayName( user_id );

            return std::shared_ptr<TwitchUser>( new TwitchUser{ this->parent(),
                                     query.value( "user_id" ).toInt(),
                                     name,
                                     displayname,
                                     query.value( "last_update" ).toString(),
                                     query.value( "created_at" ).toString(),
                                     query.value( "updated_at" ).toString(),
                                     query.value( "bio" ).toString(),
                                     query.value( "logo" ).toString(),
                                     query.value( "type" ).toString(),
                                     query.value( "notifications" ).toInt() } );
        }
    }
    else
    {
        qDebug() << "SqLite error:" << query.lastError().text();
    }

    return nullptr;
}

void DB_Manager::addFollowerCountHistory( const QString &created_at, const int &follower_count )
{
    QSqlQuery query( this->db );

    query.prepare( "INSERT INTO follower_count_history(created_at, follower_count) "
                   "VALUES(:created_at, :follower_count )" );

    query.bindValue( ":created_at", created_at );
    query.bindValue( ":follower_count", follower_count );

    if( !query.exec() )
    {
        qDebug() << "SqLite error:" << query.lastError().text();
    }
}

void DB_Manager::addUserName( const int &user_id, const QString &username, const QString &created_at ) const
{
    QSqlQuery query( this->db );

    query.prepare( "INSERT INTO twitch_user_name_history(user_id,name,created_at) "
                   "VALUES(:user_id,:name,:created_at)" );

    query.bindValue( ":user_id", user_id );
    query.bindValue( ":name", username );
    query.bindValue( ":created_at", created_at );

    if( !query.exec() )
    {
        qDebug() << "SqLite error:" << query.lastError().text();
    }
}

*/

bool DB_Manager::isOk() const
{
    if( this->tableExists( "settings" ) &&
        this->tableExists( "languages" ) &&
        this->tableExists( "words" ) &&
        this->tableExists( "translations" ) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

QVector<QString> DB_Manager::getLangs() const
{
    QVector<QString> langs;

    QSqlQuery query( this->db );

    query.prepare( "SELECT * FROM languages" );

    if( query.exec() )
    {
        while( query.next() )
        {
            const QString lang = query.value( "lang" ).toString();
            langs.push_back( lang );
        }
    }
    else
    {
        ::logError( "SqLite error:" + query.lastError().text() );
        throw "SqLite error:" + query.lastError().text();
    }

    return langs;
}

int DB_Manager::getLangId( const QString &langTag ) const
{
    int langId = 0;

    QSqlQuery query( this->db );
    query.prepare( "SELECT * FROM languages WHERE lang = :langTag" );
    query.bindValue( ":langTag", langTag );

    if( query.exec() )
    {
        if( query.next() )
        {
            langId = query.value( "id" ).toInt();
        }
    }
    else
    {
        ::logError( "SqLite error:" + query.lastError().text() );
        throw "SqLite error:" + query.lastError().text();
    }

    return langId;
}

int DB_Manager::getWordId( const QString &word, const int &lang_id ) const
{
    QSqlQuery query( this->db );
    query.prepare( "SELECT * FROM words WHERE word = :word AND lang_id = :lang_id" );

    query.bindValue( ":word", word );
    query.bindValue( ":lang_id", lang_id );

    if( query.exec() )
    {
        if( query.next() )
        {
            return query.value( "id" ).toInt();
        }

        throw "Could not be! There is no current language set in table settings!!!";
    }
    else
    {
        ::logError( "SqLite error:" + query.lastError().text() );
        throw "SqLite error:" + query.lastError().text();
    }
}

QString DB_Manager::getWord( const int word_id ) const
{
    QSqlQuery query( this->db );
    query.prepare( "SELECT word FROM words WHERE id = :word_id" );

    query.bindValue( ":word_id", word_id );

    if( query.exec() )
    {
        if( query.next() )
        {
            return query.value( "word" ).toString();
        }

        throw "Could not be! There is no current language set in table settings!!!";
    }
    else
    {
        ::logError( "SqLite error:" + query.lastError().text() );
        throw "SqLite error:" + query.lastError().text();
    }
}

bool DB_Manager::isTranslatedWord( const QString &word, const int &lang_id ) const
{
    if( !this->isKnownWord( word, lang_id ) )
    {
        return false;
    }
    else
    {
        const int word_id = this->getWordId( word, lang_id );
        const int native_lang_id = this->getLangId( this->getCurrentNativeLang() );

        QSqlQuery query( this->db );
        query.prepare( "SELECT * FROM words,translations WHERE translations.from_word_id = :word_id "
                       "and translations.to_word_id=words.id and words.lang_id = :native_lang_id" );

        query.bindValue( ":word_id", word_id );
        query.bindValue( ":native_lang_id", native_lang_id );

        if( query.exec() )
        {
            if( query.next() )
            {
                return true;
            }

            return false;
        }
        else
        {
            ::logError( "SqLite error:" + query.lastError().text() );
            throw "SqLite error:" + query.lastError().text();
        }
    }
}

bool DB_Manager::isKnownWord( const QString &word, const int &lang_id ) const
{
    //const int wordId{ this->getWordId( word ) };
    //const int nativeLangId = this->getLangId( this->getCurrentNativeLang() );

    QSqlQuery query( this->db );
    query.prepare( "SELECT id FROM words WHERE word = :word AND lang_id = :lang_id" );

    query.bindValue( ":word", word );
    query.bindValue( ":lang_id", lang_id );

    if( query.exec() )
    {
        if( query.next() )
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        ::logError( "SqLite error:" + query.lastError().text() );
        throw "SqLite error:" + query.lastError().text();
    }
}

QString DB_Manager::getCurrentNativeLang() const
{
    QSqlQuery query( this->db );

    if( query.exec( "select lang from settings,languages WHERE settings.value = languages.id" ) )
    {
        if( query.next() )
        {
            const QString langTag{ query.value( "lang" ).toString() };

            if( !langTag.isEmpty() )
            {
                return langTag;
            }
        }

        throw "Could not be! There is no current language set in table settings!!!";
    }
    else
    {
        ::logError( "SqLite error:" + query.lastError().text() );
        throw "SqLite error:" + query.lastError().text();
    }
}

void DB_Manager::insertNewWord( const QString &word, const int &lang_id ) const
{
    QSqlQuery query( this->db );

    query.prepare( "INSERT INTO words(word,lang_id)"
                   "VALUES(:word,:lang_id)" );

    query.bindValue( ":word", word );
    query.bindValue( ":lang_id", lang_id );

    if( !query.exec() )
    {
        ::logError( "SqLite error:" + query.lastError().text() );
        throw "SqLite error:" + query.lastError().text();
    }
}

void DB_Manager::translate( const QString &nativeWord, const int &nativeLangId,
                            const QString &foreignWord, const int &foreignLangId ) const
{
    QSqlQuery query( this->db );

    if( !this->isKnownWord( nativeWord, nativeLangId ) )
    {
        this->insertNewWord( nativeWord, nativeLangId );
    }

    if( !this->isKnownWord( foreignWord, foreignLangId ) )
    {
        this->insertNewWord( foreignWord, foreignLangId );
    }

    const int from_word_id = this->getWordId( nativeWord, nativeLangId );
    const int to_word_id = this->getWordId( foreignWord, foreignLangId );

    query.prepare( "INSERT INTO translations(from_word_id,to_word_id)"
                   "VALUES(:from_word_id,:to_word_id)" );

    query.bindValue( ":from_word_id", from_word_id );
    query.bindValue( ":to_word_id", to_word_id );

    if( !query.exec() )
    {
        ::logError( "SqLite error:" + query.lastError().text() );
        throw "SqLite error:" + query.lastError().text();
    }
}

QVector<QString> DB_Manager::getTanslations( const QString &from_word, const int &foreign_lang_id,
                                             const int &native_lang_id ) const
{
    QSqlQuery query( this->db );

    const QString sql = "SELECT translations.to_word_id FROM translations "
            "WHERE translations.from_word_id = (SELECT id from words "
            "WHERE word = :from_word AND lang_id = :foreign_lang_id)";

    qDebug() << sql;

    query.prepare( sql );

    //query.bindValue( ":native_lang_id", native_lang_id );
    query.bindValue( ":from_word", from_word );
    query.bindValue( ":foreign_lang_id", foreign_lang_id );

    if( query.exec() )
    {
        qDebug() << "query exec ok";

        QVector<QString> transations;

        while( query.next() )
        {
            qDebug() << "query result yes";

            const int word_id = query.value( "to_word_id" ).toInt();

            //qDebug() << word_id ;
            const QString sql2 = "SELECT word FROM words WHERE words.lang_id = :native_lang_id AND words.id = :word_id";

            QSqlQuery query2( this->db );
            query2.prepare( sql2 );

            query2.bindValue( ":native_lang_id", native_lang_id );
            query2.bindValue( ":word_id", word_id );

            if( query2.exec() )
            {
                if( query2.next() )
                {
                    const QString word = query2.value( "word" ).toString();

                    if( !word.isEmpty() )
                    {
                        transations.push_back( word );
                    }
                }
            }

        }

        return transations;
    }
    else
    {
        ::logError( "SqLite error:" + query.lastError().text() );
        throw "SqLite error:" + query.lastError().text();
    }

    return QVector<QString>{};
}

void DB_Manager::update( const int wordID, const QString &word ) const
{
    QSqlQuery query( this->db );

    query.prepare( "UPDATE words SET word = :word WHERE id = :id" );

    query.bindValue( ":word", word );
    query.bindValue( ":id", wordID );

    if( !query.exec() )
    {
        ::logError( "SqLite error:" + query.lastError().text() );
        throw "SqLite error:" + query.lastError().text();
    }
}

void DB_Manager::remove( const int wordID ) const
{
    QSqlQuery query( this->db );

    query.prepare( "DELETE FROM words WHERE id = :id" );

    query.bindValue( ":id", wordID );

    if( !query.exec() )
    {
        ::logError( "SqLite error:" + query.lastError().text() );
        throw "SqLite error:" + query.lastError().text();
    }

    QSqlQuery query2( this->db );

    query2.prepare( "DELETE FROM translations WHERE from_word_id = " + QString::number(wordID )
                    +  " OR to_word_id = + " + QString::number(wordID ) );

    if( !query2.exec() )
    {
        ::logError( "SqLite error:" + query2.lastError().text() );
        throw "SqLite error:" + query2.lastError().text();
    }
}

bool DB_Manager::tableExists( const QString &tableName ) const
{
    QSqlQuery query( this->db );

    if( query.exec( "SELECT * FROM " + tableName ) )
    {
        return true;
    }
    else
    {
        return false;
    }
}
