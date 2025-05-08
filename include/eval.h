#ifndef EVAL_H // include guard
#define EVAL_H

#include "board.h"

#define PAWN_CP   100
#define KNIGHT_CP 300
#define BISHOP_CP 300
#define ROOK_CP   500
#define QUEEN_CP  900

#define CHECKMATE_CP (2 << 10)

#define MAX_DEPTH 20

typedef struct {
    Move line[MAX_DEPTH];
    bool is_mate;
    int depth;
    int score;
} PrincipleVariation;

//PrincipleVariation *start_eval(Board *board, int depth);
PrincipleVariation eval(Board *board, int depth);
int piece_eval(Board *board);
int piece_loc_eval(Board *board);

//void free_pv(PrincipleVariation *pv);


#endif // EVAL_H
