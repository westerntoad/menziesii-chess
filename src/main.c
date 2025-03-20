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

    Board *board = from_fen("8/8/4k3/8/2P1R1n1/8/8/4K3 b - - 0 1");
    print_board(board);
    legal_moves(board);
    /*Move mv = new_move(63, 55, 0b0000);
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
    wprintf(L"\n\n\n");*/
    
    //wprintf(L"\n\n%x\n\n", ~((Move) 1<<27));

    /*U64 orig = 1ULL << 34;
    U64 moves = n_moves(orig);
    print_bb(moves);
    while (moves) {
        wprintf(L"\nPOPPED\n");
        print_bb(pop_lsb(&moves));
        wprintf(L"\n");
        print_bb(moves);
    }*/
}
