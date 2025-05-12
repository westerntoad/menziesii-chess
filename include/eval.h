#ifndef EVAL_H // include guard
#define EVAL_H

#include "board.h"

#define PAWN_CP   100
#define KNIGHT_CP 300
#define BISHOP_CP 300
#define ROOK_CP   500
#define QUEEN_CP  900

#define MAX_DEPTH 20

typedef struct {
    Move line[MAX_DEPTH];
    bool is_mate;
    int depth;
    int score;
    int alpha;
    int beta;
} PrincipleVariation;

PrincipleVariation eval(Board *board, int depth);
int piece_eval(Board *board);

#endif // EVAL_H
