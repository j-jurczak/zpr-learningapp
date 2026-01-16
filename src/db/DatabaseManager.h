/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class DatabaseManager, manages database connections and operations - header file.
 */
#pragma once
#include <QSqlDatabase>
#include <string>
#include <vector>
#include <optional>
#include <tuple>

#include "../core/learning/Card.h"
#include "../core/learning/StudySet.h"

struct DraftCard {
    std::string question;
    std::string correct_answer;
    std::vector<std::string> wrong_answers;
};

class DatabaseManager {
public:
    explicit DatabaseManager( const QString& db_name = "learning_app.db" );
    ~DatabaseManager();

    bool connect();
    bool createTables();
    void seedData();
    void flushData();

    std::vector<StudySet> getAllSets();
    std::optional<StudySet> getSet( int set_id );
    std::vector<Card> getCardsForSet( int set_id );

    bool createSet( const std::string& set_name, const std::vector<DraftCard>& cards );
    bool deleteSet( int set_id );
    bool addCardToSet( int set_id, const DraftCard& card );
    bool deleteCard( int card_id );

    std::vector<Card> getRandomCards( int set_id, int limit );
    std::vector<Card> getDueCards( int set_id, int limit );
    std::tuple<int, int, float> getCardProgress( int card_id );
    bool updateCardProgress( int card_id, int interval, int repetitions, float easiness,
                             const std::string& next_date );
    static std::string calculateNextDate( int days_from_now );

private:
    QSqlDatabase database_;
    QString db_name_;
    std::vector<Card> getCardsWithQuery( const QString& query_str, int set_id, int limit );
};