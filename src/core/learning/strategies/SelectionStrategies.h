/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Concrete implementations of selection strategies.
 */
#pragma once
#include "ICardSelectionStrategy.h"

class RandomSelectionStrategy : public ICardSelectionStrategy {
public:
    std::vector<Card> selectCards( DatabaseManager& db, int set_id, int limit ) override {
        return db.getRandomCards( set_id, limit );
    }
};

class SpacedRepetitionStrategy : public ICardSelectionStrategy {
public:
    std::vector<Card> selectCards( DatabaseManager& db, int set_id, int limit ) override {
        return db.getDueCards( set_id, limit );
    }
};