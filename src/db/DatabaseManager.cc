/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class DatabaseManager, manages database connections and operations - source file.
 */
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>
#include <QCoreApplication>
#include <QDir>
#include <QDate>

#include "DatabaseManager.h"

using namespace std;

DatabaseManager::DatabaseManager( const QString& db_name ) : db_name_( db_name ) {}

DatabaseManager::~DatabaseManager() {
    if ( database_.isOpen() ) {
        database_.close();
    }
}

// establishes connection to the SQLite database
bool DatabaseManager::connect() {
    if ( QSqlDatabase::contains( QSqlDatabase::defaultConnection ) ) {
        database_ = QSqlDatabase::database( QSqlDatabase::defaultConnection );
    } else {
        database_ = QSqlDatabase::addDatabase( "QSQLITE" );
    }
    QString root;
#ifdef PROJECT_ROOT
    root = QString( PROJECT_ROOT );
#else
    root = QDir::currentPath();
#endif
    data_path_ = root + "/data";
    QDir dir( data_path_ );
    if ( !dir.exists() ) {
        if ( dir.mkpath( "." ) ) {
            qDebug() << "Data directory created at:" << data_path_;
        } else {
            qCritical() << "Error: Could not create data directory";
        }
    }
    if ( !dir.exists( "media/images" ) ) dir.mkpath( "media/images" );
    if ( !dir.exists( "media/sounds" ) ) dir.mkpath( "media/sounds" );

    QString dbPath = data_path_ + "/" + db_name_;

    qDebug() << "Database path:" << dbPath;
    database_.setDatabaseName( dbPath );

    if ( !database_.open() ) {
        qCritical() << "Error: connection with database failed:" << database_.lastError().text();
        return false;
    }
    return true;
}

// creates necessary tables if they do not exist
bool DatabaseManager::createTables() {
    QSqlQuery query;

    bool sets_ok = query.exec(
        "CREATE TABLE IF NOT EXISTS sets ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL"
        ")" );

    bool cards_ok = query.exec(
        "CREATE TABLE IF NOT EXISTS cards ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "set_id INTEGER NOT NULL, "
        "question TEXT NOT NULL, "  // either text or path to media
        "correct_answer TEXT NOT NULL, "
        "wrong_answers TEXT, "
        "media_type INTEGER DEFAULT 0, "   // Enum MediaType
        "answer_type INTEGER DEFAULT 0, "  // Enum AnswerType
        "FOREIGN KEY(set_id) REFERENCES sets(id) ON DELETE CASCADE"
        ")" );

    bool progress_ok = query.exec(
        "CREATE TABLE IF NOT EXISTS learning_progress ("
        "card_id INTEGER PRIMARY KEY, "
        "interval INTEGER DEFAULT 0, "
        "repetitions INTEGER DEFAULT 0, "
        "easiness_factor REAL DEFAULT 2.5, "
        "next_review_date TEXT, "
        "FOREIGN KEY(card_id) REFERENCES cards(id) ON DELETE CASCADE"
        ")" );

    return sets_ok && cards_ok && progress_ok;
}

// seeding database with initial data for testing
void DatabaseManager::seedData() {
    QSqlQuery check( "SELECT COUNT(*) FROM sets" );
    if ( check.next() && check.value( 0 ).toInt() > 0 ) return;

    qDebug() << "Seeding database with initial data...";
    QSqlQuery q;

    q.exec( "INSERT INTO sets (name) VALUES ('Angielski Podstawy')" );
    int set_id = q.lastInsertId().toInt();

    auto insertCard = [&]( const QString& ques, const QString& ans, const QString& wrongs,
                           int a_type, int m_type ) {
        q.prepare(
            "INSERT INTO cards (set_id, question, correct_answer, wrong_answers, answer_type, "
            "media_type) "
            "VALUES (:sid, :q, :a, :w, :atype, :mtype)" );
        q.bindValue( ":sid", set_id );
        q.bindValue( ":q", ques );
        q.bindValue( ":a", ans );
        q.bindValue( ":w", wrongs );
        q.bindValue( ":atype", a_type );
        q.bindValue( ":mtype", m_type );
        q.exec();
    };

    insertCard( "Pies", "Dog", "", 0, 0 );

    insertCard( "Kot", "Cat", "", 4, 0 );

    insertCard( "Kolor nieba?", "Blue", "Red;Green;Yellow", 1, 0 );
    insertCard( "'Chicken' to po polsku...", "Kurczak", "Indyk;Kaczka;Gęś", 1, 0 );
}

// deletes all data from the database
void DatabaseManager::flushData() {
    QSqlQuery q;
    q.exec( "DELETE FROM learning_progress" );
    q.exec( "DELETE FROM cards" );
    q.exec( "DELETE FROM sets" );
}

// select query for all study sets
vector<StudySet> DatabaseManager::getAllSets() const {
    vector<StudySet> results;
    QSqlQuery query( "SELECT id, name FROM sets ORDER BY id DESC" );

    while ( query.next() ) {
        StudySet s;
        s.id = query.value( "id" ).toInt();
        s.name = query.value( "name" ).toString().toStdString();

        QSqlQuery count_q;
        count_q.prepare( "SELECT COUNT(*) FROM cards WHERE set_id = :id" );
        count_q.bindValue( ":id", s.id );
        if ( count_q.exec() && count_q.next() ) s.card_count = count_q.value( 0 ).toInt();

        results.push_back( s );
    }
    return results;
}

// select query for a specific study set by id
optional<StudySet> DatabaseManager::getSet( int set_id ) const {
    QSqlQuery query;
    query.prepare( "SELECT id, name FROM sets WHERE id = :id" );
    query.bindValue( ":id", set_id );

    if ( query.exec() && query.next() ) {
        StudySet s;
        s.id = query.value( "id" ).toInt();
        s.name = query.value( "name" ).toString().toStdString();
        return s;
    }
    return nullopt;
}

// select query for all cards in a given set
vector<Card> DatabaseManager::getCardsForSet( int set_id ) const {
    QString sql =
        "SELECT id, set_id, question, correct_answer, wrong_answers, answer_type, media_type "
        "FROM cards WHERE set_id = :id";
    return getCardsWithQuery( sql, set_id, -1 );
}

// retrieved random cards
vector<Card> DatabaseManager::getRandomCards( int set_id, int limit ) const {
    QString sql =
        "SELECT id, set_id, question, correct_answer, wrong_answers, answer_type, media_type "
        "FROM cards WHERE set_id = :id ORDER BY RANDOM() LIMIT :limit";
    return getCardsWithQuery( sql, set_id, limit );
}

// retrieved cards due for review (SM-2 logic)
vector<Card> DatabaseManager::getDueCards( int set_id, int limit ) const {
    QString sql = R"(
        SELECT c.id, c.set_id, c.question, c.correct_answer, c.wrong_answers, c.answer_type, c.media_type 
        FROM cards c
        LEFT JOIN learning_progress lp ON c.id = lp.card_id
        WHERE c.set_id = :id
          AND (lp.next_review_date IS NULL OR lp.next_review_date <= date('now', 'localtime'))
        ORDER BY lp.next_review_date ASC
        LIMIT :limit
    )";
    return getCardsWithQuery( sql, set_id, limit );
}

// retrieves learning progress for a specific card
tuple<int, int, float> DatabaseManager::getCardProgress( int card_id ) const {
    QSqlQuery query;
    query.prepare(
        "SELECT interval, repetitions, easiness_factor FROM learning_progress WHERE card_id = "
        ":id" );
    query.bindValue( ":id", card_id );

    if ( query.exec() && query.next() ) {
        return { query.value( 0 ).toInt(), query.value( 1 ).toInt(), query.value( 2 ).toFloat() };
    }
    return { 0, 0, 2.5f };
}

QString DatabaseManager::getImagesPath() const { return data_path_ + "/media/images/"; }

QString DatabaseManager::getSoundsPath() const { return data_path_ + "/media/sounds/"; }

// insert query to create a new set with its cards
bool DatabaseManager::createSet( const string& set_name, const vector<DraftCard>& cards ) {
    if ( set_name.empty() ) return false;

    database_.transaction();

    QSqlQuery query;
    query.prepare( "INSERT INTO sets (name) VALUES (:name)" );
    query.bindValue( ":name", QString::fromStdString( set_name ) );

    if ( !query.exec() ) {
        qCritical() << "Could not add set:" << query.lastError().text();
        database_.rollback();
        return false;
    }

    int new_set_id = query.lastInsertId().toInt();

    for ( const auto& card : cards ) {
        if ( !addCardToSet( new_set_id, card ) ) {
            database_.rollback();
            return false;
        }
    }

    return database_.commit();
}

// delete query to remove a set by id
bool DatabaseManager::deleteSet( int set_id ) {
    database_.transaction();
    QSqlQuery query;

    query.prepare(
        "DELETE FROM learning_progress WHERE card_id IN (SELECT id FROM cards WHERE set_id = "
        ":id)" );
    query.bindValue( ":id", set_id );
    if ( !query.exec() ) {
        qCritical() << "Failed to delete learning progress for set:" << set_id
                    << query.lastError().text();
        database_.rollback();
        return false;
    }

    query.prepare( "DELETE FROM cards WHERE set_id = :id" );
    query.bindValue( ":id", set_id );
    if ( !query.exec() ) {
        qCritical() << "Failed to delete cards for set:" << set_id << query.lastError().text();
        database_.rollback();
        return false;
    }

    query.prepare( "DELETE FROM sets WHERE id = :id" );
    query.bindValue( ":id", set_id );
    if ( !query.exec() ) {
        qCritical() << "Could not delete set ID:" << set_id
                    << " Error:" << query.lastError().text();
        database_.rollback();
        return false;
    }

    return database_.commit();
}

// insert query to add a single card to an existing set
bool DatabaseManager::addCardToSet( int set_id, const DraftCard& card ) {
    QSqlQuery query;
    query.prepare(
        "INSERT INTO cards (set_id, question, correct_answer, wrong_answers, media_type, "
        "answer_type) "
        "VALUES (:sid, :q, :a, :w, :mtype, :atype)" );

    QString wrong_str = "";
    if ( !card.wrong_answers.empty() ) {
        QStringList wrong_list;
        for ( const auto& wa : card.wrong_answers ) {
            wrong_list << QString::fromStdString( wa );
        }
        wrong_str = wrong_list.join( ";" );
    }

    query.bindValue( ":sid", set_id );
    query.bindValue( ":q", QString::fromStdString( card.question ) );
    query.bindValue( ":a", QString::fromStdString( card.correct_answer ) );
    query.bindValue( ":w", wrong_str );
    query.bindValue( ":mtype", static_cast<int>( card.media_type ) );
    query.bindValue( ":atype", static_cast<int>( card.answer_type ) );

    if ( !query.exec() ) {
        qCritical() << "Error adding single card:" << query.lastError().text();
        return false;
    }
    return true;
}

// delete query to remove a card by id
bool DatabaseManager::deleteCard( int card_id ) {
    QSqlQuery query;
    query.prepare( "DELETE FROM cards WHERE id = :id" );
    query.bindValue( ":id", card_id );

    if ( !query.exec() ) {
        qCritical() << "Could not delete card ID:" << card_id
                    << " Error:" << query.lastError().text();
        return false;
    }
    return true;
}

// updates learning progress for a specific card
bool DatabaseManager::updateCardProgress( int card_id, int interval, int repetitions,
                                          float easiness, const string& next_date ) {
    QSqlQuery query;
    query.prepare( R"(
        INSERT OR REPLACE INTO learning_progress (card_id, interval, repetitions, easiness_factor, next_review_date)
        VALUES (:id, :iv, :rep, :ef, :date)
    )" );

    query.bindValue( ":id", card_id );
    query.bindValue( ":iv", interval );
    query.bindValue( ":rep", repetitions );
    query.bindValue( ":ef", easiness );
    query.bindValue( ":date", QString::fromStdString( next_date ) );

    if ( !query.exec() ) {
        qCritical() << "Error saving progress:" << query.lastError().text();
        return false;
    }
    return true;
}

// Clears learning progress for all cards in a set
bool DatabaseManager::resetSetProgress( int set_id ) {
    QSqlQuery query;
    query.prepare(
        "DELETE FROM learning_progress WHERE card_id IN (SELECT id FROM cards WHERE set_id = "
        ":id)" );
    query.bindValue( ":id", set_id );

    if ( !query.exec() ) {
        qCritical() << "Failed to reset progress:" << query.lastError().text();
        return false;
    }
    return true;
}

// calculates the next review date based on the current date and a given offset
string DatabaseManager::calculateNextDate( int days_from_now ) {
    return QDate::currentDate().addDays( days_from_now ).toString( "yyyy-MM-dd" ).toStdString();
}

vector<Card> DatabaseManager::getCardsWithQuery( const QString& sql, int set_id, int limit ) const {
    vector<Card> cards;
    QSqlQuery query;
    query.prepare( sql );
    query.bindValue( ":id", set_id );
    if ( limit > 0 ) query.bindValue( ":limit", limit );

    if ( !query.exec() ) {
        qCritical() << "Error executing card query:" << query.lastError().text();
        return cards;
    }

    while ( query.next() ) {
        int id = query.value( "id" ).toInt();
        int sid = query.value( "set_id" ).toInt();

        string q_str = query.value( "question" ).toString().toStdString();
        string a_str = query.value( "correct_answer" ).toString().toStdString();
        QString w_raw = query.value( "wrong_answers" ).toString();
        int m_val = query.value( "media_type" ).toInt();
        int a_val = query.value( "answer_type" ).toInt();

        MediaType m_type = static_cast<MediaType>( m_val );
        AnswerType a_type = static_cast<AnswerType>( a_val );

        CardData data;
        data.correct_answer = a_str;
        data.answer_type = a_type;

        if ( !w_raw.isEmpty() ) {
            for ( const auto& part : w_raw.split( ';', Qt::SkipEmptyParts ) ) {
                data.wrong_answers.push_back( part.toStdString() );
            }
        }
        switch ( m_type ) {
            case MediaType::IMAGE:
                data.question = ImageContent{ q_str };
                break;
            case MediaType::SOUND:
                data.question = SoundContent{ q_str };
                break;
            case MediaType::TEXT:
            default:
                data.question = TextContent{ q_str };
                break;
        }
        cards.emplace_back( id, sid, move( data ), m_type );
    }
    return cards;
}