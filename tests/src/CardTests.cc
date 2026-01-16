#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <algorithm>
#include <iostream>

#include "../../src/core/learning/Card.h"
#include "../../src/core/learning/CardTypes.h"

using namespace std;

TEST_CASE( "Card Class Functionality", "[Card]" ) {
    CardData flashcard_data;
    flashcard_data.question = TextContent{ "What is the largest planet?" };
    flashcard_data.correct_answer = "Jupiter";
    flashcard_data.answer_type = AnswerType::FLASHCARD;

    Card standard_card( 1, 100, flashcard_data, MediaType::TEXT );

    CardData quiz_data;
    quiz_data.question = ImageContent{ "stop_sign.png" };
    quiz_data.correct_answer = "Red";
    quiz_data.wrong_answers = { "Blue", "Green", "Yellow" };
    quiz_data.answer_type = AnswerType::TEXT_CHOICE;

    Card image_quiz_card( 2, 200, quiz_data, MediaType::IMAGE );

    CardData input_data;
    input_data.question = TextContent{ "Translate: Kot" };
    input_data.correct_answer = "Cat";
    input_data.answer_type = AnswerType::INPUT;

    Card input_card( 3, 300, input_data, MediaType::TEXT );

    SECTION( "Constructor and Getters (Text Content)" ) {
        REQUIRE( standard_card.getId() == 1 );
        REQUIRE( standard_card.getMediaType() == MediaType::TEXT );

        REQUIRE( standard_card.getQuestion() == "What is the largest planet?" );

        REQUIRE( standard_card.getData().answer_type == AnswerType::FLASHCARD );
    }

    SECTION( "Constructor and Getters (Image Content)" ) {
        REQUIRE( image_quiz_card.getId() == 2 );
        REQUIRE( image_quiz_card.getMediaType() == MediaType::IMAGE );

        REQUIRE( image_quiz_card.getQuestion() == "stop_sign.png" );
    }

    SECTION( "Answer Checking - Case Insensitivity" ) {
        CHECK( standard_card.checkAnswer( "Jupiter" ) );
        CHECK( standard_card.checkAnswer( "JUPITER" ) );
        CHECK( standard_card.checkAnswer( "jupiter" ) );
        CHECK_FALSE( standard_card.checkAnswer( "Mars" ) );

        CHECK( input_card.checkAnswer( "Cat" ) );
        CHECK( input_card.checkAnswer( "cat" ) );
        CHECK_FALSE( input_card.checkAnswer( "Dog" ) );
    }

    SECTION( "Is Choice Card Logic" ) {
        REQUIRE( image_quiz_card.isChoiceCard() );

        REQUIRE_FALSE( standard_card.isChoiceCard() );
        REQUIRE_FALSE( input_card.isChoiceCard() );
    }

    SECTION( "Multiple Choice Handling (Shuffling)" ) {
        REQUIRE( standard_card.getChoices().empty() );

        vector<string> choices = image_quiz_card.getChoices();

        REQUIRE( choices.size() == 4 );

        REQUIRE_FALSE( find( choices.begin(), choices.end(), "Red" ) == choices.end() );
        REQUIRE_FALSE( find( choices.begin(), choices.end(), "Blue" ) == choices.end() );
        REQUIRE_FALSE( find( choices.begin(), choices.end(), "Green" ) == choices.end() );
        REQUIRE_FALSE( find( choices.begin(), choices.end(), "Yellow" ) == choices.end() );
    }
}