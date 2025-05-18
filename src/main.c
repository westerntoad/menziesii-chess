#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "uci.h"

#include "utils.h"

// TODO
// 50 half-move rule
// three-fold repitition
// value passed pawns
// undervalue isolated pawns
// add pondering
// mobility score for individual pieces
// resrict quiescent search depth
// write README
// tablebases
// iterative deepening
// better move reording

int main(void) {
    srand(time(NULL));
    setbuf(stdout, NULL);

    return uci();
}
