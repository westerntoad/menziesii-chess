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
// position startpos moves d2d4 g8f6 e2e3 e7e6 b1c3 f8e7 g1f3 e8g8 e3e4 d7d5 e4e5 f6e8 f1d3 c7c5 e1g1 b8c6 d4c5 e7c5 c3a4 c5e7 a4c3 e7b4 c3e2 d8c7 a2a3 b4a5 d3b5 c6e5 f3e5 c7e5 b5e8 f8e8 e2d4 a5b6 c1e3 f7f5 f1e1 e5f6 f2f4 g8h8 a3a4 g7g5 f4g5 f6g6 a4a5 b6d4 d1d4 h8g8 c2c4 e6e5 d4d5 g6e6 a1d1 f5f4 e3c1 e6d5 d1d5 c8e6 d5e5 g8f7 e5e6 e8e6 c1f4 e6e1
// go depth 5

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
