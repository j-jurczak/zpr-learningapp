/*
 * @authors: Jakub Jurczak, Mateusz Woźniak
 * summary: SuperMemo-2 algorithm math logic - source file.
 */
#include <cmath>

#include "SuperMemo.h"

using namespace std;

SuperMemoState SuperMemo::calculate( int grade, const SuperMemoState& current_state ) {
    int next_interval = 0;
    int next_rep = 0;
    float next_ef = current_state.easiness;

    // If the grade is greater than or equal to 3, the review is considered successful
    if ( grade >= 3 ) {
        if ( current_state.repetitions == 0 )
            next_interval = 1;
        else if ( current_state.repetitions == 1 )
            next_interval = 6;
        else
            next_interval =
                static_cast<int>( ceil( current_state.interval * current_state.easiness ) );

        next_rep = current_state.repetitions + 1;

        // from the formula: EF':=EF+(0.1-(5-q)*(0.08+(5-q)*0.02))
        next_ef = current_state.easiness +
                  ( 0.1f - ( 5.0f - grade ) * ( 0.08f + ( 5.0f - grade ) * 0.02f ) );
        if ( next_ef < MIN_EASINESS ) next_ef = MIN_EASINESS;
    } else {
        next_rep = 0;
        next_interval = 1;
    }

    return { next_interval, next_rep, next_ef };
}
