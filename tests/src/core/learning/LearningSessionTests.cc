#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <QFile>
#include <QDir>
#include <memory>
#include <iostream>
#include <variant>

#include "core/learning/LearningSession.h"
#include "core/learning/strategies/SelectionStrategies.h"
#include "db/DatabaseManager.h"
#include "core/learning/Card.h"

using namespace std;

Card create_test_card( int id, const string& question ) {
    CardData data;
    data.id = id;
    data.set_id = 1;
    data.question = TextContent{ question };
    data.correct_answer = "Answer";
    data.answer_type = AnswerType::FLASHCARD;

    return Card( data );
}

class MockSelectionStrategy : public ICardSelectionStrategy {
    vector<Card> cards_to_return;

public:
    MockSelectionStrategy( vector<Card> cards ) : cards_to_return( cards ) {}
    vector<Card> selectCards( DatabaseManager&, int, int ) override { return cards_to_return; }
};

TEST_CASE( "LearningSession Integration Tests", "[LearningSession]" ) {
    QString db_path = "test_learning.db";

#ifdef PROJECT_ROOT
    QString root = QString( PROJECT_ROOT );
    db_path = root + "/data/test_learning.db";
#endif

    if ( QFile::exists( db_path ) ) {
        if ( !QFile::remove( db_path ) ) {
            cerr << "Warning: Could not remove old test DB: " << db_path.toStdString() << endl;
        }
    }

    DatabaseManager db( "test_learning.db" );

    if ( !db.connect() ) {
        FAIL( "Could not connect to TEST database." );
    }
    db.createTables();

    db.createSet( "Integration Test Set", {} );

    auto add_db_card = [&]( const string& q, const string& a ) {
        DraftCard draft;
        draft.question = TextContent{ q };
        draft.correct_answer = a;
        draft.answer_type = AnswerType::FLASHCARD;
        db.addCardToSet( 1, draft );
    };

    add_db_card( "Q1", "A1" );
    add_db_card( "Q2", "A2" );
    add_db_card( "Q3", "A3" );

    vector<Card> memory_cards = { create_test_card( 1, "Q1" ), create_test_card( 2, "Q2" ),
                                  create_test_card( 3, "Q3" ) };

    SECTION( "Linear Flow with Mock Strategy" ) {
        LearningSession session( db );
        session.start( 1, make_unique<MockSelectionStrategy>( memory_cards ) );

        REQUIRE( session.getCurrentCard().getId() == 1 );
        session.submitGrade( 5 );

        REQUIRE( session.nextCard() );
        REQUIRE( session.getCurrentCard().getId() == 2 );
        session.submitGrade( 4 );

        REQUIRE( session.nextCard() );
        REQUIRE( session.getCurrentCard().getId() == 3 );
        session.submitGrade( 5 );

        REQUIRE_FALSE( session.nextCard() );
        REQUIRE_THAT( session.getProgress(), Catch::Matchers::WithinAbs( 1.0f, 0.01f ) );

        auto [iv, rep, ef] = db.getCardProgress( 1 );
        REQUIRE( iv == 1 );
        REQUIRE( rep == 1 );
    }

    SECTION( "Spaced Repetition Loop (Retry in Session)" ) {
        LearningSession session( db );
        vector<Card> single_card = { create_test_card( 1, "Hard One" ) };

        session.start( 1, make_unique<MockSelectionStrategy>( single_card ) );

        session.submitGrade( 1 );

        REQUIRE( session.nextCard() == true );
        REQUIRE( session.getCurrentCard().getId() == 1 );

        auto [iv, rep, ef] = db.getCardProgress( 1 );
        REQUIRE( iv == 1 );
        REQUIRE( rep == 0 );

        session.submitGrade( 4 );
        REQUIRE_FALSE( session.nextCard() );
    }
}