#include <string.h>
#include "eval.h"
#include "utils.h"
#include "types.h"

#define CHECKMATE_CP (2 << 15)
#define POS_INF (2 << 16)
#define NEG_INF (2 << 16)*(-1)

extern volatile int STOP_SEARCH;
extern U64 NUM_NODES;

const int PIECE_VALUES[] = {
    PAWN_CP, KNIGHT_CP, BISHOP_CP,
    ROOK_CP, QUEEN_CP, 0
};

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

PrincipleVariation eval(Board *board, int depth) {
    // WHITE: side_coeff = 1
    // BLACK: side_coeff = -1
    int side_coeff = (board->side_to_move * 2 - 1)*(-1);
    PrincipleVariation pv;
    PrincipleVariation delta_pv;
    memset(pv.line, 0, MAX_DEPTH * sizeof(Move));
    pv.is_mate = false;
    pv.depth = depth;
    pv.score = POS_INF*side_coeff*(-1);

    NUM_NODES++;

    if (depth > 0) {
        Move *curr = (Move[256]){0};
        Move *end = legal_moves(board, curr);

        if (curr == end) {
            pv.depth = 0;
            if (is_in_check(board)) {
                // mate
                pv.is_mate = true;
                pv.score = CHECKMATE_CP * side_coeff*(-1);
            } else {
                // stalemate
                pv.score = 0;
            }
            return pv;
        }

        while (curr < end && (depth < 3 || !STOP_SEARCH)) {
            make_move(board, *curr);
            delta_pv = eval(board, depth-1);
            if ((board->side_to_move == WHITE && delta_pv.score < pv.score) || (board->side_to_move == BLACK && delta_pv.score > pv.score)) {
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
        pv.score = piece_eval(board);
    }

    return pv;
}



int piece_eval(Board *board) {
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
                if (i == 1) // don't ask
                    pc = flip_v(pc);
                val += (PIECE_SQUARE_TABLE[j][pc] + PIECE_VALUES[j]) * i;
            }
        }

        side++;
    }

    return val;
}
