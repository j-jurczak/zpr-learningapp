/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class representing a learning card - source file.
 */
#include <algorithm>
#include <cctype>

#include "Card.h"

using namespace std;

Card::Card( const CardData& data ) : data_( data ) {
    if ( holds_alternative<ImageContent>( data_.question ) ) {
        media_type_ = MediaType::IMAGE;
    } else if ( holds_alternative<SoundContent>( data_.question ) ) {
        media_type_ = MediaType::SOUND;
    } else {
        media_type_ = MediaType::TEXT;
    }
}

bool Card::checkAnswer( string_view user_answer ) const {
    return areStringsEqual( user_answer, data_.correct_answer );
}

vector<string> Card::getChoices() const {
    vector<string> choices = data_.wrong_answers;
    choices.push_back( data_.correct_answer );
    return choices;
}

string Card::getQuestion() const {
    if ( holds_alternative<TextContent>( data_.question ) ) {
        return get<TextContent>( data_.question ).text;
    }
    return "[Media Content]";
}

bool Card::isChoiceCard() const {
    return !data_.wrong_answers.empty() || data_.answer_type == AnswerType::TEXT_CHOICE ||
           data_.answer_type == AnswerType::IMAGE_CHOICE;
}

bool Card::areStringsEqual( string_view a, string_view b ) {
    return equal( a.begin(), a.end(), b.begin(), b.end(),
                  []( char a, char b ) { return tolower( a ) == tolower( b ); } );
}