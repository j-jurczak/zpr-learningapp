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
    AnswerType answer_type = AnswerType::FLASHCARD;
    MediaType media_type = MediaType::TEXT;
};

class DatabaseManager {
public:
    explicit DatabaseManager( const QString& db_name = "learning_app.db" );
    ~DatabaseManager();

    bool connect();
    bool createTables();
    void seedData();
    void flushData();

    std::vector<StudySet> getAllSets() const;
    std::optional<StudySet> getSet( int set_id ) const;
    std::vector<Card> getCardsForSet( int set_id ) const;
    std::vector<Card> getRandomCards( int set_id, int limit ) const;
    std::vector<Card> getDueCards( int set_id, int limit ) const;
    std::tuple<int, int, float> getCardProgress( int card_id ) const;
    QString getImagesPath() const;
    QString getSoundsPath() const;

    bool createSet( const std::string& set_name, const std::vector<DraftCard>& cards );
    bool deleteSet( int set_id );
    bool addCardToSet( int set_id, const DraftCard& card );
    bool deleteCard( int card_id );

    bool updateCardProgress( int card_id, int interval, int repetitions, float easiness,
                             const std::string& next_date );
    bool resetSetProgress( int set_id );
    static std::string calculateNextDate( int days_from_now );

private:
    QSqlDatabase database_;
    QString db_name_;
    QString data_path_;

    std::vector<Card> getCardsWithQuery( const QString& query_str, int set_id, int limit ) const;
};