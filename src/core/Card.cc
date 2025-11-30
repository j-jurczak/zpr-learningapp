/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class representing a learning card - source file.
 */
#include "Card.h"

#include <algorithm>
#include <cctype>
#include <random>

Card::Card( int id, int set_id, std::string question, CardData data, MediaType media )
    : id_( id ),
      set_id_( set_id ),
      question_( std::move( question ) ),
      data_( std::move( data ) ),
      media_type_( media ) {}

bool Card::checkAnswer( std::string_view user_answer ) const {
    auto is_same = []( std::string_view a, std::string_view b ) {
        return std::equal( a.begin(), a.end(), b.begin(), b.end(),
                           []( char a, char b ) { return tolower( a ) == tolower( b ); } );
    };

    return std::visit(
        [&]( const auto& concrete_data ) -> bool {
            return is_same( user_answer, concrete_data.correct_answer );
        },
        data_ );
}

std::vector<std::string> Card::getChoices() const {
    if ( const ChoiceData* choice = std::get_if<ChoiceData>( &data_ ) ) {
        std::vector<std::string> all = choice->wrong_answers;
        all.push_back( choice->correct_answer );

        std::random_device rd;
        std::default_random_engine rng( rd() );
        std::shuffle( all.begin(), all.end(), rng );

        return all;
    }

    return {};
}

bool Card::isChoiceCard() const { return std::holds_alternative<ChoiceData>( data_ ); }