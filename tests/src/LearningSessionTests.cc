#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <string>
#include "../../src/core/LearningSession.h"

using namespace std;

Card create_standard_card( int id, const string& answer ) {
    StandardData data = { answer };
    return Card( id, 1, "Question " + to_string( id ), data, MediaType::TEXT );
}

TEST_CASE( "LearningSession initialization and basic flow", "[LearningSession]" ) {
    vector<Card> initial_cards = { create_standard_card( 1, "Answer A" ),
                                   create_standard_card( 2, "Answer B" ),
                                   create_standard_card( 3, "Answer C" ) };
    const size_t num_cards = initial_cards.size();

    SECTION( "Default constructor and loadCards" ) {
        LearningSession session;

        session.loadCards( initial_cards );

        session.start();

        REQUIRE( session.getCurrentCard().getId() == 1 );
    }

    SECTION( "Card navigation" ) {
        LearningSession session( initial_cards );
        session.start();

        REQUIRE( session.getCurrentCard().getId() == 1 );

        REQUIRE( session.nextCard() );
        REQUIRE( session.getCurrentCard().getId() == 2 );

        REQUIRE( session.nextCard() );
        REQUIRE( session.getCurrentCard().getId() == 3 );

        REQUIRE_FALSE( session.nextCard() );
        REQUIRE( session.getCurrentCard().getId() == 3 );
    }

    SECTION( "getCurrentCard throws exception when session empty" ) {
        LearningSession empty_session;
        empty_session.start();
        REQUIRE_THROWS_AS( empty_session.getCurrentCard(), out_of_range );
    }
}