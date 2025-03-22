#include <locale.h> // used for unicode printing
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "uci.h"
//#include "board.h"
#include "movegen.h"
#include "utils.h" // includes <stdio.h>

#define WIDE_PRINT false
#define MAX_STR_SIZE 32

int main(void) {
    char *str = (char*)malloc(MAX_STR_SIZE * sizeof(char));

    if (WIDE_PRINT) {
        setlocale(LC_CTYPE, ""); // used for unicode printing
        // ensure stdout is wide-oriented
        if (fwide(stdout, 0) <= 0) {
            fwide(stdout, 1);
        }
    }
    init_move_lookup_tables();

    /*Board *board = from_fen("7k/8/8/2R3r1/8/8/8/K7 w - - 0 1");
    Move move = new_move(c5,g5,4);
    make_move(board, move);
    unmake_move(board, move);
    print_board(board);*/
    //print_board_bb(board);
    //Move* moves = legal_moves(board);
    //print_move_buffer(moves);
    //return 0;

    while (1) {
        if (fgets(str, MAX_STR_SIZE, stdin) == NULL)
            return -1;

        if (strcmp(str, "uci\n") == 0)
            return uci();
    }
}
