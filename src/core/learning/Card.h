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

struct TextContent {
    std::string text;
};

struct ImageContent {
    std::string image_path;
};

struct SoundContent {
    std::string sound_path;
};

using QuestionPayload = std::variant<TextContent, ImageContent, SoundContent>;

struct CardData {
    QuestionPayload question;
    std::string correct_answer;
    std::vector<std::string> wrong_answers;
    AnswerType answer_type = AnswerType::FLASHCARD;
};

class Card {
public:
    Card( int id, int set_id, CardData data, MediaType media_type );

    Card() = default;

    bool checkAnswer( std::string_view user_answer ) const;
    const std::string& getCorrectAnswer() const;
    std::vector<std::string> getChoices() const;

    // getters
    int getId() const { return id_; }
    int getSetId() const { return set_id_; }
    MediaType getMediaType() const { return media_type_; }
    const CardData& getData() const { return data_; }
    std::string getQuestion() const;

    bool isChoiceCard() const;

private:
    int id_;
    int set_id_;

    MediaType media_type_;
    CardData data_;

    static bool areStringsEqual( std::string_view a, std::string_view b );
};