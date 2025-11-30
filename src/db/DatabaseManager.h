/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class DatabaseManager, manages database connections and operations - header file.
 */

#pragma once
#include <QSqlDatabase>
#include <string>
#include <vector>

#include "../core/Card.h"
#include "../core/StudySet.h"

class DatabaseManager {
public:
    DatabaseManager();

    bool connect();
    bool createTables();
    void seedData();

    std::vector<StudySet> getAllSets();
    std::vector<Card> getCardsForSet( int set_id );

private:
    QSqlDatabase database_;
};