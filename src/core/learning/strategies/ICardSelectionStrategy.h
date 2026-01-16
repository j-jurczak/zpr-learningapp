/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Interface for card selection strategies.
 */
#pragma once
#include <vector>

#include "../Card.h"
#include "../../../db/DatabaseManager.h"

class ICardSelectionStrategy {
public:
    virtual ~ICardSelectionStrategy() = default;

    virtual std::vector<Card> selectCards( DatabaseManager& db, int set_id, int limit ) = 0;
};