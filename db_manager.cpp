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
    }

    return langs;
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
