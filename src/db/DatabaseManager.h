/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class DatabaseManager, manages database connections and operations - header file.
 */
#pragma once
#include <QSqlDatabase>
#include <string>
#include <vector>
#include <optional>

#include "../core/Card.h"
#include "../core/StudySet.h"

struct DraftCard {
    std::string question;
    std::string correct_answer;
    std::vector<std::string> wrong_answers;
};

class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();

    bool connect();
    bool createTables();
    void seedData();
    void flushData();

    std::vector<StudySet> getAllSets();
    std::optional<StudySet> getSet( int set_id );
    std::vector<Card> getCardsForSet( int set_id );

    bool createSet( const std::string& set_name, const std::vector<DraftCard>& cards );

private:
    QSqlDatabase database_;
};