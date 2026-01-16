/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: SuperMemo-2 algorithm math logic - header file.
 */
#pragma once

struct SuperMemoState {
    int interval;     // days to next review
    int repetitions;  // number of successful reviews
    float easiness;   // easiness factor (interval modifier)
};

class SuperMemo {
public:
    static constexpr int INITIAL_INTERVAL = 0;
    static constexpr int INITIAL_REPETITIONS = 0;
    static constexpr float INITIAL_EASINESS = 2.5f;
    static constexpr float MIN_EASINESS = 1.3f;

    static SuperMemoState getInitialState() {
        return { INITIAL_INTERVAL, INITIAL_REPETITIONS, INITIAL_EASINESS };
    }
    static SuperMemoState calculate( int grade, const SuperMemoState& current );
};