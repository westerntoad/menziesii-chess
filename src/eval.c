#include <string.h>
#include "eval.h"
#include "utils.h"
#include "types.h"

// All tables were taken from:
// https://www.chessprogramming.org/Simplified_Evaluation_Function#Piece-Square_Tables
static const int PIECE_SQUARE_TABLE[NUM_PIECES + 1][NUM_SQUARES] = {
    { // PAWN
         0,  0,  0,  0,  0,  0,  0,  0,
         50, 50, 50, 50, 50, 50, 50, 50,
         10, 10, 20, 30, 30, 20, 10, 10,
         5,  5, 10, 25, 25, 10,  5,  5,
         0,  0,  0, 20, 20,  0,  0,  0,
         5, -5,-10,  0,  0,-10, -5,  5,
         5, 10, 10,-20,-20, 10, 10,  5,
         0,  0,  0,  0,  0,  0,  0,  0
    }, { // KNIGHT
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50
    }, { // BISHOP
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20
    }, { // ROOK
         0,  0,  0,  0,  0,  0,  0,  0,
         5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
         0,  0,  0,  5,  5,  0,  0,  0
    }, { // QUEEN
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -5,  0,  5,  5,  5,  5,  0, -5,
         0,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    }, { // KING MIDDLE GAME
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
         20, 20,  0,  0,  0,  0, 20, 20,
         20, 30, 10,  0,  0, 10, 30, 20
    }, { // KING END GAME
        -50,-40,-30,-20,-20,-30,-40,-50,
        -30,-20,-10,  0,  0,-10,-20,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-30,  0,  0,  0,  0,-30,-30,
        -50,-30,-30,-30,-30,-30,-30,-50        
    }
};

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
    // WHITE: side_coeff = 1
    // BLACK: side_coeff = -1
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
                pv.score = CHECKMATE_CP * side_coeff*(-1);
//printf("RECOGNIZE CHECKMATE (sc=%d)\n", pv.score);
            } else {
                // stalemate
//printf("RECOGNIZE STALEMATE\n");
                pv.score = 0;
            }
            return pv;
        }

        make_move(board, *curr);
        PrincipleVariation delta_pv = eval(board, depth-1);
        pv.score = delta_pv.score;
/*print_board(board);
printf("\nScore = %d\n\n", pv.score);*/
        pv.line[0] = *curr;
        memcpy(pv.line + 1, delta_pv.line, (MAX_DEPTH - 1) * sizeof(Move)); // maybe correct?
        pv.is_mate = delta_pv.is_mate;
        pv.depth = delta_pv.depth + 1;
        unmake_move(board, *curr);
        curr++;
        while (curr < end) {
            make_move(board, *curr);
            delta_pv = eval(board, depth-1);
/*print_board(board);
printf("\nScore = %d\n\n", delta_pv.score);*/
            if ((board->side_to_move == WHITE && delta_pv.score < pv.score) || (board->side_to_move == BLACK && delta_pv.score > pv.score)) {
                /*printf("\n");
                print_move(*curr);
                printf("(sc=%d) > ", delta_pv.score);
                print_move(pv.line[0]);
                printf("(sc=%d)\n", pv.score);*/
                pv.score = delta_pv.score;
                pv.line[0] = *curr;
                memcpy(pv.line + 1, delta_pv.line, (MAX_DEPTH - 1) * sizeof(Move)); // maybe correct?
                pv.is_mate = delta_pv.is_mate;
                pv.depth = delta_pv.depth + 1;

            }

            unmake_move(board, *curr);
            curr++;
        }
    } else {
        pv.score = piece_eval(board) + piece_loc_eval(board);
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

int piece_loc_eval(Board *board) {
    int i, j, side = 0, val = 0;
    U64 bb, color;
    Sq pc;

    for (i = 1; i >= -1; i -= 2) { // iterate over both colors
        color = board->colors[side];
        for (j = 0; j < NUM_PIECES; j++) { // iterate over all pieces
            // TODO add endgame king
            bb = board->pieces[j] & color;
            while (bb) {
                pc = LOG2(pop_lsb(&bb));
//printf("CHECKING POS:\n");
//print_bb(1ULL << pc);
                if (i == 1) // don't ask
                    pc = flip_v(pc);
//printf("\nValue = %d\n\n", PIECE_SQUARE_TABLE[j][pc] * i);
                val += PIECE_SQUARE_TABLE[j][pc] * i;
            }
        }

        side++;
    }

    return val;
}
