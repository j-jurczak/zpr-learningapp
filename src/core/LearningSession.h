/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class representing a learning session, holder of cards being learned - header file.
 */

#pragma once
#include <vector>

#include "Card.h"

class LearningSession {
public:
    LearningSession();

    void loadCards( std::vector<Card> cards );
    void start();
    const Card& getCurrentCard() const;
    bool nextCard();
    void submitGrade( int grade );
    float getProgress() const;

private:
    std::vector<Card> deck_;
    size_t current_card_index_;
};