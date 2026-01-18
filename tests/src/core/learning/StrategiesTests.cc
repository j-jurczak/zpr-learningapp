#include <catch2/catch_test_macros.hpp>
#include <QDir>
#include <iostream>

#include "core/learning/strategies/SelectionStrategies.h"
#include "db/DatabaseManager.h"

using namespace std;

TEST_CASE( "Selection Strategies", "[Strategies]" ) {
    QString test_db_name = "strat_test_db.sqlite";
    DatabaseManager db( test_db_name );
    db.connect();
    db.createTables();
    db.flushData();

    vector<DraftCard> cards;
    for( int i = 0; i < 5; i++ ) {
        DraftCard c;
        c.question = TextContent{ "Q" + to_string( i ) };
        c.correct_answer = "A";
        c.answer_type = AnswerType::FLASHCARD;
        cards.push_back( c );
    }
    db.createSet( "Strategy Set", cards );
    int set_id = db.getAllSets()[0].id;
    vector<Card> db_cards = db.getCardsForSet( set_id );

    SECTION( "Random Selection" ) {
        RandomSelectionStrategy strategy;

        vector<Card> selected = strategy.selectCards( db, set_id, 3 );
        REQUIRE( selected.size() == 3 );

        selected = strategy.selectCards( db, set_id, 10 );
        REQUIRE( selected.size() == 5 );
    }

    SECTION( "Spaced Repetition (Due Cards)" ) {
        SpacedRepetitionStrategy strategy;

        vector<Card> due = strategy.selectCards( db, set_id, 5 );

        int c_id = db_cards[0].getId();
        db.updateCardProgress( c_id, 10, 1, 2.5, DatabaseManager::calculateNextDate( 10 ) );

        due = strategy.selectCards( db, set_id, 100 );
        REQUIRE( due.size() == 4 );
    }

    QFile::remove( QDir::current().filePath( "data/" + test_db_name ) );
}
