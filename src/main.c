#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "uci.h"

#include "utils.h"

// TODO
// 50 half-move rule
// value passed pawns
// undervalue isolated pawns
// add pondering
// mobility score for individual pieces
// resrict quiescent search depth
// write README
// tablebases
// iterative deepening
// better move reording
// endgame king piece-square table
// change to a fail-soft alpha beta search
// limit on selective depth for check

// BUGS
// position fen 1k6/7R/2K5/8/8/8/8/8 b - - 2 2 
// # incorrect mate

int main(void) {
    srand(time(NULL));
    setbuf(stdout, NULL);

    return uci();
}
