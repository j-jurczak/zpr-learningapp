/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Structure representing a study set.
 */
#pragma once
#include <string>

struct StudySet {
    int id;
    std::string name;
    int card_count = 0;

    StudySet() = default;
    StudySet( int id, std::string name, int count = 0 )
        : id( id ), name( std::move( name ) ), card_count( count ) {}
};