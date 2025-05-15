#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "uci.h"

// TODO
// TRANSPOSITION TABLE
// 50 half-move rule
// three-fold repitition
// value passed pawns
// undervalue isolated pawns
// add checks to quiescent
// add pondering
// mobility score for individual pieces
// resrict quiescent search depth
// write README
// tablebases
// iterative deepening

int main(void) {
    srand(time(NULL));
    setbuf(stdout, NULL);

    return uci();
}
