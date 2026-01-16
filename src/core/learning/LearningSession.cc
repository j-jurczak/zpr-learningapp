/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Learning session implementation.
 */
#include <stdexcept>

#include "LearningSession.h"

using namespace std;

LearningSession::LearningSession( DatabaseManager& db ) : db_( db ) {}

void LearningSession::start( int set_id, unique_ptr<ICardSelectionStrategy> strategy, int limit ) {
    if ( !strategy ) {
        throw invalid_argument( "Strategy cannot be null" );
    }
    vector<Card> cards = strategy->selectCards( db_, set_id, limit );

    session_queue_.clear();
    for ( const auto& c : cards ) {
        session_queue_.push_back( c );
    }

    total_cards_initial_ = session_queue_.size();
    nextCard();
}

bool LearningSession::nextCard() {
    if ( session_queue_.empty() ) {
        current_card_.reset();
        return false;
    }

    current_card_ = session_queue_.front();
    session_queue_.pop_front();
    return true;
}

const Card& LearningSession::getCurrentCard() const {
    if ( !current_card_.has_value() ) {
        throw runtime_error( "No active card" );
    }
    return *current_card_;
}

void LearningSession::submitGrade( int grade ) {
    if ( !current_card_.has_value() ) return;

    auto [iv, rep, ef] = db_.getCardProgress( current_card_->getId() );
    SuperMemoState currentState{ iv, rep, ef };

    SuperMemoState newState = SuperMemo::calculate( grade, currentState );
    string nextDate = DatabaseManager::calculateNextDate( newState.interval );
    db_.updateCardProgress( current_card_->getId(), newState.interval, newState.repetitions,
                            newState.easiness, nextDate );

    if ( grade < 3 ) {
        session_queue_.push_back( *current_card_ );
    }
}

float LearningSession::getProgress() const {
    if ( total_cards_initial_ == 0 ) return FULL_PROGRESS;
    return FULL_PROGRESS - ( static_cast<float>( session_queue_.size() ) / total_cards_initial_ );
}