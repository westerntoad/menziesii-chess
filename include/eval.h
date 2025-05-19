#ifndef EVAL_H // include guard
#define EVAL_H

#include "board.h"

#define PAWN_CP   100
#define KNIGHT_CP 300
#define BISHOP_CP 300
#define ROOK_CP   500
#define QUEEN_CP  900

#define MAX_DEPTH 20

#define CHECKMATE_CP (2 << 18)

U64 eval(Board *board, U8 depth);
int piece_eval(Board *board);
void start_timer();

#endif // EVAL_H
