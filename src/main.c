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

    //Board *board = from_fen("8/8/4k3/8/2P1R1n1/8/8/4K3 b - - 0 1");
    //Board *board = from_fen("8/4N3/2k3R1/1P6/2K5/8/8/7B b - - 0 1");
    Board *board = from_fen("K7/8/8/8/8/R3p2k/8/8 b - - 0 1");
    print_board(board);
    /*Move mv = new_move(d2, d4, 0b0001);
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
    
    //wprintf(L"\n\n%x\n\n", 1<<15);
}
