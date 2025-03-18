#include "types.h"
#include "movegen.h"
#include "board.h"
#include "utils.h" // includes <stdio.h>

int main() {
    init_move_lookup_tables();
    Board *board = from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    print_board(board);
}
