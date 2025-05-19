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
// position fen 8/8/8/8/4K3/8/1R6/4k3 b - - 0 1 moves e1d1 e4d3 d1e7
// # can't find mate? I believe due to whose move it is originally
//
// position fen 8/8/8/8/4K3/8/1R6/4k3 b - - 0 1 moves e1d1 e4d3 d1e7 d1e1
// # something happening
//
// position fen 1k6/7R/2K5/8/8/8/8/8 b - - 2 2 
// # incorrect mate

int main(void) {
    srand(time(NULL));
    setbuf(stdout, NULL);

    return uci();
}
