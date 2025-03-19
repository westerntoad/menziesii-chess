#ifndef BOARD_H // include guard
#define BOARD_H

#include "types.h"

#define MAX_NUM_MOVES 17_697

typedef struct {
    U64  colors[NUM_COLORS];
    U64  pieces[NUM_PIECES];
    int  ply;
    bool side_to_move;
    StateFlags *state_stack;
    int move_capacity;
} Board;

void make_move(Board *board, Move move);
void unmake_move(Board *board, Move move);
bool can_castle(Board *board, bool color, bool side);
int half_moves(Board *board);
void print_board(Board *board);
Board* from_fen(char* fen);

#endif  // BOARD_H
