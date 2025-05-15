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
    char *str = (char*)malloc(MAX_STR_SIZE * sizeof(char));
    srand(time(NULL));
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
