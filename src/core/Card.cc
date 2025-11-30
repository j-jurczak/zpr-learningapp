/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class representing a learning card - source file.
 */
#include <algorithm>
#include <cctype>
#include <random>
#include "Card.h"

using namespace std;

Card::Card( int id, int set_id, string question, CardData data, MediaType media )
    : id_( id ),
      set_id_( set_id ),
      question_( move( question ) ),
      data_( move( data ) ),
      media_type_( media ) {}

bool Card::areStringsEqual( string_view a, string_view b ) {
    if ( a.size() != b.size() ) {
        return false;
    }
    return equal( a.begin(), a.end(), b.begin(), []( unsigned char c1, unsigned char c2 ) {
        return tolower( c1 ) == tolower( c2 );
    } );
}

bool Card::checkAnswer( std::string_view user_answer ) const {
    return std::visit(
        [&]( const auto& concrete_data ) {
            return areStringsEqual( user_answer, concrete_data.correct_answer );
        },
        data_ );
}

vector<string> Card::getChoices() const {
    if ( const ChoiceData* choice = get_if<ChoiceData>( &data_ ) ) {
        vector<string> all = choice->wrong_answers;
        all.push_back( choice->correct_answer );

        random_device rd;
        default_random_engine rng( rd() );
        shuffle( all.begin(), all.end(), rng );  // randomize order

        return all;
    }

    return {};
}

bool Card::isChoiceCard() const { return holds_alternative<ChoiceData>( data_ ); }