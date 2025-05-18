#include <math.h>
#include <string.h>
#include <time.h>
#include "eval.h"
#include "utils.h"
#include "table.h"
#include "types.h"

#define INF (2 << 16)

extern volatile int STOP_SEARCH;
extern volatile int SEARCH_TIME;
extern U64 NUM_NODES;
struct timespec START_TIME, END_TIME;

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

static int should_stop_search(U8 depth) {
    if (STOP_SEARCH)
        return 1;

    if (depth <= 4)
        return 0;

    if (SEARCH_TIME > 0) {
        clock_gettime(CLOCK_MONOTONIC, &END_TIME);
        double duration = (END_TIME.tv_sec - START_TIME.tv_sec);
        duration += (END_TIME.tv_nsec - START_TIME.tv_nsec) / 1000000000.0;
        //SEARCH_TIME -= (int)round(duration*1000);
        if ((int)round(duration*1000) >= SEARCH_TIME) {
            STOP_SEARCH = 1; // TODO not atomic
        }
    }
    
    return 0;
}

/*void reorder_moves(Move* list, int n, Move pv) {
    // TODO better reordering
    if (!pv)
        return;

    Move temp = *list;
    int i;

    for (i = 0; i < n && list[i] != pv; i++);

    list[0] = pv;
    list[i] = temp;
}*/

int quiesce(Board *board, int alpha, int beta) {
    int score, best = piece_eval(board);

    NUM_NODES++;

    if (best >= beta)
        return best;

    if (best > alpha)
        alpha = best;

    Move *curr = (Move[256]){0};
    Move *end = legal_moves(board, curr);

    while (curr < end) {
        if (!is_capture(*curr)) {
            curr++;
            continue;
        }
        make_move(board, *curr);
        score = -quiesce(board, -beta, -alpha);
        unmake_move(board, *curr);

        if (score > best)
            best = score;

        if (best >= beta)
            return best;

        if (score > alpha)
            alpha = score;

        curr++;
    }

    return best;
}

int alphabeta(Board *board, int alpha, int beta, U8 depth) {
    bool preempted = false;
    NUM_NODES++;
    
    if (depth == 0)
        return quiesce(board, alpha, beta);

    TTEntry* tt_entry = tt_probe(board->hash);
    if (tt_entry && tt_entry->depth >= depth) {
        if (tt_entry->type == EXACT_NODE) {
            return tt_entry->score;
        } else if (tt_entry->type == ALL_NODE && tt_entry->score <= alpha) {
            return alpha;
        } else if (tt_entry->type == CUT_NODE  && tt_entry->score >= beta) {
            return beta;
        }
    }

    Move *curr = (Move[256]){0};
    Move *end = legal_moves(board, curr);

    if (curr == end) {
        if (is_in_check(board)) {
            // mate
            return -(CHECKMATE_CP + depth); // TODO this is bad
        } else {
            // stalemate
            return 0; // TODO contempt score
        }
    }

    /*if (tt_entry)
        reorder_moves(curr, end - curr, tt_entry->best);*/

    Move best_move = *curr;
    char flag = ALL_NODE;

    while (curr < end) {
        if (should_stop_search(depth)) {
            preempted = true;
            break;
        }

        make_move(board, *curr);
        int score = -alphabeta(board, -beta, -alpha, depth-1);
        unmake_move(board, *curr);

        if (score >= beta) {
            tt_save(board->hash, depth, beta, best_move, CUT_NODE);
            return beta;
        }
        
        if (score > alpha) {
            flag = EXACT_NODE;
            best_move = *curr;
            alpha = score;
        }
        
        curr++;
    }

    if (!preempted)
        tt_save(board->hash, depth, alpha, best_move, flag);

    return alpha;
}

void start_timer() {
    clock_gettime(CLOCK_MONOTONIC, &START_TIME);
}

U64 eval(Board *board, U8 depth) {
    alphabeta(board, -INF, INF, depth);
    return board_hash(board);
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

    // return the value relative to side to move (required for negamax)
    return val * (board->side_to_move * 2 - 1)*(-1);
}
