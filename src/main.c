#include <locale.h> // used for unicode printing
#include "types.h"
#include "movegen.h"
#include "board.h"
#include "utils.h" // includes <stdio.h>

int main() {
    setlocale(LC_CTYPE, ""); // used for unicode printing
    // ensure stdout is wide-oriented
    if (fwide(stdout, 0) <= 0) {
        fwide(stdout, 1);
    }

    init_move_lookup_tables();

    Board *board = from_fen("7k/3r4/8/8/3N4/8/8/K7 b - - 0 1");
    Move mv = new_move(51, 27, 0b0100);
    print_board(board);
    make_move(board, mv);
    wprintf(L"\nAFTER MAKE_MOVE ");
    print_move(mv);
    wprintf(L"\n");
    print_board(board);
    unmake_move(board, mv);
    wprintf(L"\nAFTER UNMAKE_MOVE ");
    print_move(mv);
    wprintf(L"\n");
    print_board(board);
    wprintf(L"\n\n\n");
}
