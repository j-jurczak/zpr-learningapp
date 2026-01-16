/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Enums describing card types.
 */
#pragma once

enum class MediaType { TEXT, IMAGE, SOUND };

enum class AnswerType {
    FLASHCARD,  // click to reveal answer
    CHOICE,     // click one of the given answers
    INPUT       // write answer
};