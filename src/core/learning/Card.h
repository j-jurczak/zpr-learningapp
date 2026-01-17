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
    int id;
    int set_id;
    QuestionPayload question;
    std::string correct_answer;
    std::vector<std::string> wrong_answers;
    AnswerType answer_type = AnswerType::FLASHCARD;
};

struct DraftCard {
    QuestionPayload question;
    std::string correct_answer;
    std::vector<std::string> wrong_answers;
    AnswerType answer_type = AnswerType::FLASHCARD;
};

class Card {
public:
    explicit Card( const CardData& data );

    Card() = default;

    bool checkAnswer( std::string_view user_answer ) const;
    std::vector<std::string> getChoices() const;
    std::string getQuestion() const;
    bool isChoiceCard() const;

    int getId() const { return data_.id; }
    int getSetId() const { return data_.set_id; }
    MediaType getMediaType() const { return media_type_; }
    const CardData& getData() const { return data_; }
    const std::string& getCorrectAnswer() const { return data_.correct_answer; }

private:
    MediaType media_type_;
    CardData data_;

    static bool areStringsEqual( std::string_view a, std::string_view b );
};