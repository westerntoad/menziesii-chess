#include <locale.h> // used for unicode printing
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "uci.h"
#include "board.h"
#include "movegen.h"
#include "utils.h" // includes <stdio.h>

#define WIDE_PRINT false
#define MAX_STR_SIZE 32

// CHECK THESE
// position startpos moves e2e4 b8c6 d2d4 d7d5 e4d5 d8d5 g1f3 d5e4 d1e2 e4e2 f1e2 c6b4 b1a3 g8f6 c2c3 b4d5 e2c4 c8g4 f3e5 e7e6 h2h3 f8a3 b2a3 g4f5 g2g4 f5e4 f2f3 e4g6 h3h4 d5b6 h4h5 b6c4 e5c4 g6c2 e1d2 c2a4 a1b1 a4c6 c4e5 e8g8 e5c6 b7c6 b1b7 f8c8 a3a4 f6d5 d2d3 c8b8 b7b8 a8b8 c1a3 b8e8 d3c4 a7a6 a3b4 e8b8 a2a3 a6a5 b4a5 b8a8 a5b4 a8a4 c4b3 a4a6 h1b1 a6a8 b4c5 a8b8 b3c2 b8b1 c2b1 d5c3 b1c2 c3e2 c2b3 e2g1 f3f4 g1e2 f4f5 e6f5 g4f5 e2g3 a3a4 g3h5 a4a5 h5g3 a5a6 h7h6 a6a7 g8h7 a7a8q g3f5 a8c6 g7g6 c6c7 h7g7 c7e5 g7h7 d4d5 h7g8 e5e8 g8g7 c5b4
// go
//
// position fen 4Q3/5pk1/6pp/3P1n2/1B6/1K6/8/8 b - - 0 52
// go


// position startpos moves d2d4 g8f6 e2e3 g7g6 b1c3 f8g7 d4d5 e7e5 e3e4 e8g8 g1f3 d7d6 f1d3 b8d7 e1g1 d7c5 h2h3 a7a5 c1e3 b7b6 e3c5 b6c5 f1e1 f6h5 d1c1 h5f4 d3c4 f7f5 c1e3 g7h6 e4f5 f4h3 g2h3 h6e3 e1e3 g6f5 g1h2 e5e4 f3d2 d8g5 a1g1 g5g1 h2g1 f8f6 e3g3 g8f7 c3b5 f5f4 g3c3 c8f5 b5c7 a8g8 g1h1 f6h6 c4f1 g8g5 d2c4 f7e7 c4a5 g5h5 a5c6 e7d7 c7b5 f5h3 f1h3 h5h3 c3h3 h6h3 h1g1 e4e3 c6b8 d7e7 f2e3 f4e3 b8c6 e7f6 b5d6 e3e2 g1f2 h3h2 f2e1 h7h5 d6e4 f6f7 e4c5 h5h4 d5d6 f7e8 d6d7 e8f7 d7d8q h4h3 d8c7 f7f6 c7h2 f6f5 h2h3 f5f6 h3g4 f6f7 c5d7 f7e8 e1e2 e8f7 e2d1 f7e8 d1c1 e8f7 c1b1 f7e8 b1a1 e8f7 a1b1 f7e8
// # invalid mate

// position startpos moves d2d4 e7e6 b1c3 d7d5 e2e4 h7h6 f1b5 c7c6 b5d3 b7b5 g1f3 f8d6 e1g1 c6c5 d3b5 e8f 8 d4c5 d6c5 e4d5 e6d5 d1d5 d8d5 c3d5 b8d7 b5d7 c8d7 f3e5 d7e6 d5c7 a8c8 c7e6 f7e6 e5g6 f8e8 g6h8 c8b8 h8g6 g8f6 g6f4 e6e5 f4e6 f6e4 e6g7 e8f7 g7f5 e4f6 f5h6 f7g7 f1e1 b8e8 c1g5 c5b4 g5f6 g7f6 h6g4 f6e7 e1e5 e7d7 g4f6 d7c8 e5e8 c8b7 a1d1 a7a5 e8e6 a5a4 d1d7 b7a8
// go
// # not mating

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

int main(void) {
    char *str = (char*)malloc(MAX_STR_SIZE * sizeof(char));
    srand(time(NULL));
    init_move_lookup_tables();
    setbuf(stdout, NULL);

    while (1) {
        if (fgets(str, MAX_STR_SIZE, stdin) == NULL)
            return -1;

        if (strcmp(str, "uci\n") == 0) {
            free(str);
            return uci();
        }
    }
}
