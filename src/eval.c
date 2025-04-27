#include "eval.h"
#include "utils.h"
#include "types.h"

#define MAX_DEPTH 20

void free_pv(PrincipleVariation *pv) {
    free(pv->line);
    free(pv);
}

PrincipleVariation *start_eval(Board *board, int depth) {
    PrincipleVariation *pv = malloc(sizeof(PrincipleVariation));
    pv->line = calloc(depth, sizeof(Move));
    pv->is_mate = false;
    pv->max_depth = depth;
    pv->score = eval(board, pv, depth);

    return pv;
}

int eval(Board *board, PrincipleVariation *pv, int depth) {
    int side_coeff = (board->side_to_move * 2 - 1)*(-1);
    if (depth > 0) {
        Move *curr = (Move[256]){0};
        Move *end = legal_moves(board, curr);

        if (curr == end) {
            if (is_in_check(board)) {
                // mate
                return CHECKMATE_CP * side_coeff*(-1);
            } else {
                // stalemate
                return 0;
            }
        }

        make_move(board, *curr);
        int move_val = eval(board, pv, depth-1);
        int best_score = move_val;
        pv->line[pv->max_depth - (depth - 1) - 1] = *curr;
        if (move_val * side_coeff*(-1) == CHECKMATE_CP) {
            pv->depth = pv->max_depth;// TODO CHANGE
            pv->is_mate = true;
        } else {
            pv->depth = pv->max_depth;
            pv->is_mate = false;
        }
printf("\n\nLINE: ");
for (int i = 0; i < pv->depth; i++) {
    print_move(pv->line[i]);
    printf(" ");
}
printf("\n");
print_board(board);
        unmake_move(board, *curr);
        curr++;
        while (curr < end) {
            make_move(board, *curr);
            int move_val = eval(board, pv, depth-1);
            if (move_val * side_coeff*(-1) > best_score) {
                pv->line[pv->max_depth - (depth - 1) - 1] = *curr;

                if (move_val * side_coeff*(-1) == CHECKMATE_CP) {
                    pv->depth = pv->max_depth;// TODO CHANGE
                    pv->is_mate = true;
                } else {
                    pv->depth = pv->max_depth;
                    pv->is_mate = false;
                }
            }

            unmake_move(board, *curr);
            curr++;
        }

        return best_score;
    }

    return piece_eval(board);
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
