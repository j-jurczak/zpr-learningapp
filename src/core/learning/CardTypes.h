/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: Enums describing card types.
 */
#pragma once

enum class MediaType { TEXT, IMAGE, SOUND };

enum class AnswerType {
    FLASHCARD,     // click to reveal answer
    TEXT_CHOICE,   // click one of the given text answers
    SOUND_CHOICE,  // click one of the given answers with sound
    IMAGE_CHOICE,  // click one of the given answers with image
    INPUT          // write answer
};