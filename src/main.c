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

    Board *board = from_fen("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1");
    Move mv = new_move(0, 0, 0b0011);
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
    
    //wprintf(L"\n\n%x\n\n", ~((Move)(0b11 << 27)));
}
