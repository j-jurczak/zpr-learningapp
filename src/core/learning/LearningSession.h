/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Learning session managing card flow and logic.
 */
#pragma once
#include <vector>
#include <deque>
#include <memory>
#include <optional>

#include "Card.h"
#include "../../db/DatabaseManager.h"
#include "strategies/ICardSelectionStrategy.h"
#include "SuperMemo.h"

enum class LearningMode { SpacedRepetition, Random };

class LearningSession {
public:
    explicit LearningSession( DatabaseManager& db );

    void start( int set_id, std::unique_ptr<ICardSelectionStrategy> strategy, int limit = 20 );

    bool nextCard();
    const Card& getCurrentCard() const;
    void submitGrade( int grade );

    float getProgress() const;
    static constexpr float NO_PROGRESS = 0.0f;
    static constexpr float FULL_PROGRESS = 1.0f;

private:
    DatabaseManager& db_;
    std::deque<Card> session_queue_;
    std::optional<Card> current_card_;
    int total_cards_initial_ = 0;
};