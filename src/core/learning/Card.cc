/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class representing a learning card - source file.
 */
#include <algorithm>
#include <cctype>
#include <random>

#include "Card.h"
#include "../utils/Overloaded.h"

using namespace std;

Card::Card( int id, int set_id, CardData data, MediaType media )
    : id_( id ), set_id_( set_id ), media_type_( media ), data_( move( data ) ) {}

bool Card::areStringsEqual( string_view a, string_view b ) {
    if ( a.size() != b.size() ) {
        return false;
    }
    return equal( a.begin(), a.end(), b.begin(), []( unsigned char c1, unsigned char c2 ) {
        return tolower( c1 ) == tolower( c2 );
    } );
}

bool Card::checkAnswer( string_view user_answer ) const {
    return areStringsEqual( user_answer, data_.correct_answer );
}

const string& Card::getCorrectAnswer() const { return data_.correct_answer; }

vector<string> Card::getChoices() const {
    if ( data_.wrong_answers.empty() ) {
        return {};
    }
    vector<string> all = data_.wrong_answers;
    all.push_back( data_.correct_answer );

    static random_device rd;
    static mt19937 rng( rd() );
    shuffle( all.begin(), all.end(), rng );
    return all;
}

string Card::getQuestion() const {
    return visit( overloaded{ []( const TextContent& c ) { return c.text; },
                              []( const ImageContent& c ) { return c.image_path; },
                              []( const SoundContent& c ) { return c.sound_path; } },
                  data_.question );
}

bool Card::isChoiceCard() const { return !data_.wrong_answers.empty(); }