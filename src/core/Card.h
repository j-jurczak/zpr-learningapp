/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Class representing a learning card - header file.
 */
#pragma once
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "CardTypes.h"

struct StandardData {
    std::string correct_answer;
};

struct ChoiceData {
    std::string correct_answer;
    std::vector<std::string> wrong_answers;
};

using CardData = std::variant<StandardData, ChoiceData>;

class Card {
public:
    Card( int id, int set_id, std::string question, CardData data,
          MediaType media = MediaType::TEXT );

    Card() = default;

    bool checkAnswer( std::string_view user_answer ) const;

    std::vector<std::string> getChoices() const;

    int getId() const { return id_; }
    const std::string& getQuestion() const { return question_; }
    MediaType getMediaType() const { return media_type_; }

    bool isChoiceCard() const;

private:
    int id_;
    int set_id_;
    std::string question_;
    MediaType media_type_;
    CardData data_;
};