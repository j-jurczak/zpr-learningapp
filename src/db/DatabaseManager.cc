/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class DatabaseManager, manages database connections and operations - source file.
 */
#include "DatabaseManager.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>
#include <QCoreApplication>
#include <QDir>

using namespace std;

DatabaseManager::DatabaseManager() {}

DatabaseManager::~DatabaseManager() {
    if ( database_.isOpen() ) {
        database_.close();
    }
}

// establishes connection to the SQLite database
bool DatabaseManager::connect() {
    QDir dir( QCoreApplication::applicationDirPath() );
    dir.cdUp();
    dir.cdUp();
    QString db_path = dir.absolutePath() + "/data/learning_app.db";

    database_ = QSqlDatabase::addDatabase( "QSQLITE" );
    database_.setDatabaseName( db_path );

    if ( !database_.open() ) {
        qCritical() << "Error opening database:" << database_.lastError().text();
        return false;
    }
    return createTables();
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
        "question TEXT NOT NULL, "
        "correct_answer TEXT NOT NULL, "
        "wrong_answers TEXT, "
        "media_type INTEGER DEFAULT 0, "
        "answer_type INTEGER DEFAULT 0, "
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

    // standard card
    q.prepare(
        "INSERT INTO cards (set_id, question, correct_answer, media_type, answer_type) VALUES "
        "(:sid, :q, :a, 0, 0)" );
    q.bindValue( ":sid", set_id );
    q.bindValue( ":q", "Pies" );
    q.bindValue( ":a", "Dog" );
    q.exec();

    // choice cards
    q.prepare(
        "INSERT INTO cards (set_id, question, correct_answer, wrong_answers, answer_type) VALUES "
        "(:sid, :q, :a, :w, 1)" );

    q.bindValue( ":sid", set_id );
    q.bindValue( ":q", "Kolor nieba?" );
    q.bindValue( ":a", "Blue" );
    q.bindValue( ":w", "Red;Green;Yellow" );
    q.exec();

    q.bindValue( ":sid", set_id );
    q.bindValue( ":q", "'Chicken' to po polsku..." );
    q.bindValue( ":a", "Kurczak" );
    q.bindValue( ":w", "Indyk;Kaczka;Gęś" );
    q.exec();

    q.bindValue( ":sid", set_id );
    q.bindValue( ":q", "'Trousers' to po polsku..." );
    q.bindValue( ":a", "Spodnie" );
    q.bindValue( ":w", "Krótkie spodenki;Buty;Piżama" );
    q.exec();
}

// deletes all data from the database
void DatabaseManager::flushData() {
    QSqlQuery q;
    q.exec( "DELETE FROM learning_progress" );
    q.exec( "DELETE FROM cards" );
    q.exec( "DELETE FROM sets" );
}

// select query for all study sets
vector<StudySet> DatabaseManager::getAllSets() {
    vector<StudySet> results;
    QSqlQuery query( "SELECT id, name FROM sets ORDER BY id DESC" );

    while ( query.next() ) {
        StudySet s;
        s.id = query.value( "id" ).toInt();
        s.name = query.value( "name" ).toString().toStdString();
        results.push_back( s );
    }
    return results;
}

// select query for a specific study set by id
optional<StudySet> DatabaseManager::getSet( int set_id ) {
    QSqlQuery query;
    query.prepare( "SELECT id, name FROM sets WHERE id = :id" );
    query.bindValue( ":id", set_id );

    if ( query.exec() && query.next() ) {
        StudySet s;
        s.id = query.value( "id" ).toInt();
        s.name = query.value( "name" ).toString().toStdString();
        return s;
    }
    return std::nullopt;
}

// select query for all cards in a given set
vector<Card> DatabaseManager::getCardsForSet( int set_id ) {
    vector<Card> cards;
    QSqlQuery query;
    query.prepare( "SELECT * FROM cards WHERE set_id = :id" );
    query.bindValue( ":id", set_id );
    query.exec();

    while ( query.next() ) {
        int id = query.value( "id" ).toInt();
        string q_text = query.value( "question" ).toString().toStdString();
        string ans_text = query.value( "correct_answer" ).toString().toStdString();
        int type_val = query.value( "answer_type" ).toInt();

        CardData data;

        if ( type_val == 1 ) {  // CHOICE
            QString raw_wrong = query.value( "wrong_answers" ).toString();
            QStringList parts = raw_wrong.split( ";", Qt::SkipEmptyParts );
            vector<string> wrong_vec;
            for ( const auto& p : parts ) wrong_vec.push_back( p.toStdString() );
            data = ChoiceData{ ans_text, wrong_vec };
        } else {  // STANDARD
            data = StandardData{ ans_text };
        }

        cards.emplace_back( id, set_id, q_text, move( data ) );
    }
    return cards;
}

// insert query to create a new set with its cards
bool DatabaseManager::createSet( const std::string& set_name,
                                 const std::vector<DraftCard>& cards ) {
    if ( set_name.empty() ) return false;

    database_.transaction();

    QSqlQuery query;
    query.prepare( "INSERT INTO sets (name) VALUES (:name)" );
    query.bindValue( ":name", QString::fromStdString( set_name ) );

    if ( !query.exec() ) {
        qCritical() << "Nie udalo sie dodac zestawu:" << query.lastError().text();
        database_.rollback();
        return false;
    }

    int new_set_id = query.lastInsertId().toInt();

    query.prepare(
        "INSERT INTO cards (set_id, question, correct_answer, wrong_answers, media_type, "
        "answer_type) "
        "VALUES (:sid, :q, :a, :w, 0, :type)" );

    for ( const auto& card : cards ) {
        QString q_str = QString::fromStdString( card.question );
        QString correct_str = QString::fromStdString( card.correct_answer );
        QString wrong_str = "";
        int answer_type = 0;

        if ( !card.wrong_answers.empty() ) {
            answer_type = 1;
            QStringList wrong_list;
            for ( const auto& wa : card.wrong_answers ) {
                wrong_list << QString::fromStdString( wa );
            }
            wrong_str = wrong_list.join( ";" );
        }

        query.bindValue( ":sid", new_set_id );
        query.bindValue( ":q", q_str );
        query.bindValue( ":a", correct_str );
        query.bindValue( ":w", wrong_str );
        query.bindValue( ":type", answer_type );

        if ( !query.exec() ) {
            qCritical() << "Nie udalo sie dodac karty:" << query.lastError().text();
            database_.rollback();
            return false;
        }
    }

    return database_.commit();
}