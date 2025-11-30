#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <algorithm>
#include "../../src/core/Card.h"

using namespace std;

TEST_CASE( "Card Class Functionality", "[Card]" ) {
    StandardData standard_data = { "Jupiter" };
    Card standard_card =
        Card( 1, 100, "What is the largest planet?", standard_data, MediaType::TEXT );

    ChoiceData choice_data = { "Red", { "Blue", "Green", "Yellow" } };
    Card choice_card = Card( 2, 200, "What color is a stop sign?", choice_data, MediaType::IMAGE );

    SECTION( "Constructor and Getters" ) {
        REQUIRE( standard_card.getId() == 1 );
        REQUIRE( standard_card.getQuestion() == "What is the largest planet?" );
        REQUIRE( standard_card.getMediaType() == MediaType::TEXT );

        REQUIRE( choice_card.getId() == 2 );
        REQUIRE( choice_card.getQuestion() == "What color is a stop sign?" );
        REQUIRE( choice_card.getMediaType() == MediaType::IMAGE );
    }

    SECTION( "Answer Checking - Standard Card" ) {
        CHECK( standard_card.checkAnswer( "Jupiter" ) );

        CHECK( standard_card.checkAnswer( "JUPITER" ) );
        CHECK( standard_card.checkAnswer( "jupiter" ) );
        CHECK( standard_card.checkAnswer( "jUpiTeR" ) );

        CHECK_FALSE( standard_card.checkAnswer( "Mars" ) );

        CHECK_FALSE( standard_card.checkAnswer( "Jupit" ) );
    }

    SECTION( "Answer Checking - Choice Card" ) {
        CHECK( choice_card.checkAnswer( "Red" ) );

        CHECK( choice_card.checkAnswer( "RED" ) );
        CHECK( choice_card.checkAnswer( "rEd" ) );

        CHECK_FALSE( choice_card.checkAnswer( "Blue" ) );
    }

    SECTION( "Card Type Identification" ) {
        REQUIRE( choice_card.isChoiceCard() );
        REQUIRE_FALSE( standard_card.isChoiceCard() );
    }

    SECTION( "Multiple Choice Handling" ) {
        REQUIRE( standard_card.getChoices().empty() );

        vector<string> choices = choice_card.getChoices();

        REQUIRE( choices.size() == 4 );

        REQUIRE_FALSE( find( choices.begin(), choices.end(), "Red" ) == choices.end() );
        REQUIRE_FALSE( find( choices.begin(), choices.end(), "Blue" ) == choices.end() );
        REQUIRE_FALSE( find( choices.begin(), choices.end(), "Green" ) == choices.end() );
        REQUIRE_FALSE( find( choices.begin(), choices.end(), "Yellow" ) == choices.end() );
    }
}