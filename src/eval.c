#include <string.h>
#include "eval.h"
#include "utils.h"
#include "types.h"

/*void free_pv(PrincipleVariation *pv) {
    free(pv->line);
    free(pv);
}*/

/*PrincipleVariation *start_eval(Board *board, int depth) {
    PrincipleVariation *pv = malloc(sizeof(PrincipleVariation));
    pv->line = calloc(depth, sizeof(Move));
    pv->is_mate = false;
    pv->max_depth = depth;
    pv->score = eval(board, pv, depth);

    return pv;
}*/

PrincipleVariation eval(Board *board, int depth) {
    int side_coeff = (board->side_to_move * 2 - 1)*(-1);
    PrincipleVariation pv;
    memset(pv.line, 0, MAX_DEPTH * sizeof(Move));
    pv.is_mate = false;
    pv.depth = depth;
    if (depth > 0) {
        Move *curr = (Move[256]){0};
        Move *end = legal_moves(board, curr);

        if (curr == end) {
            pv.depth = 0;
            if (is_in_check(board)) {
                // mate
                pv.is_mate = true;
                pv.score = CHECKMATE_CP * side_coeff;
            } else {
                // stalemate
printf("RECOGNIZE STALEMATE\n");
                pv.score = 0;
            }
            return pv;
        }

        make_move(board, *curr);
        PrincipleVariation delta_pv = eval(board, depth-1);
        pv.score = delta_pv.score;
        pv.line[0] = *curr;
        memcpy(pv.line + 1, delta_pv.line, (MAX_DEPTH - 1) * sizeof(Move)); // maybe correct?
        pv.is_mate = delta_pv.is_mate;
        pv.depth = delta_pv.depth + 1;
        unmake_move(board, *curr);
        curr++;
        while (curr < end) {
            make_move(board, *curr);
            delta_pv = eval(board, depth-1);
            if (delta_pv.score * side_coeff*(-1) > pv.score) {
                pv.line[0] = *curr;
                memcpy(pv.line + 1, delta_pv.line, (MAX_DEPTH - 1) * sizeof(Move)); // maybe correct?
                pv.is_mate = delta_pv.is_mate;
                pv.depth = delta_pv.depth + 1;

            }

            unmake_move(board, *curr);
            curr++;
        }
    } else {
        pv.score = piece_eval(board);
    }

    return pv;
}

int piece_eval(Board *board) {
    int i, val = 0;
    int side = 0;
    U64 color;
    
    for (i = 1; i >= -1; i -= 2) {
        color = board->colors[side];

        val += i * POP_COUNT(board->pieces[PAWN_IDX] & color) * PAWN_CP;
        val += i * POP_COUNT(board->pieces[KNIGHT_IDX] & color) * KNIGHT_CP;
        val += i * POP_COUNT(board->pieces[BISHOP_IDX] & color) * BISHOP_CP;
        val += i * POP_COUNT(board->pieces[ROOK_IDX] & color) * ROOK_CP;
        val += i * POP_COUNT(board->pieces[QUEEN_IDX] & color) * QUEEN_CP;
        
        side++;
    }

    return val;
}
