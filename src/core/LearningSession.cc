/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class representing a learning session, holder of cards being learned - source file.
 */

#include "LearningSession.h"

#include <stdexcept>

LearningSession::LearningSession() : current_card_index_( 0 ) {}

void LearningSession::loadCards( std::vector<Card> cards ) {
    deck_ = cards;
    current_card_index_ = 0;
}

void LearningSession::start() {
    // to do: implement shuffling
    current_card_index_ = 0;
}

const Card& LearningSession::getCurrentCard() const {
    if ( current_card_index_ >= deck_.size() ) {
        throw std::out_of_range( "No card in session." );
    }
    return deck_[current_card_index_];
}

bool LearningSession::nextCard() {
    if ( current_card_index_ < deck_.size() - 1 ) {
        current_card_index_++;
        return true;
    }
    return false;
}

void LearningSession::submitGrade( int grade ) {
    // to do: implement SM-2 algorithm integration
}

float LearningSession::getProgress() const {
    if ( deck_.empty() ) return 0.0f;
    return static_cast<float>( current_card_index_ ) / deck_.size();
}