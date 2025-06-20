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
// position fen r3kb1r/ppp2p1p/2n4p/4p3/3qN1B1/3P1Q2/PPP4P/R4R1K b kq - 1 16
// # walking into mate?

int main(void) {
    srand(time(NULL));
    setbuf(stdout, NULL);

    return uci();
}
