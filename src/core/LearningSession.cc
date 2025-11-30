/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class representing a learning session, holder of cards being learned - source file.
 */
#include <stdexcept>
#include "LearningSession.h"

using namespace std;

LearningSession::LearningSession() : current_card_index_( 0 ) {}

LearningSession::LearningSession( std::vector<Card> cards )
    : deck_( move( cards ) ), current_card_index_( 0 ) {}

void LearningSession::loadCards( vector<Card> cards ) {
    deck_ = move( cards );
    current_card_index_ = 0;
}

void LearningSession::start() {
    // to do: implement shuffling / algorithm for card order
    current_card_index_ = 0;
}

const Card& LearningSession::getCurrentCard() const {
    if ( current_card_index_ >= deck_.size() ) {
        throw out_of_range( "No card in session." );
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
    // to do: implement grading logic
}

float LearningSession::getProgress() const {
    if ( deck_.empty() ) return 0.0f;
    return static_cast<float>( current_card_index_ ) / deck_.size();
}