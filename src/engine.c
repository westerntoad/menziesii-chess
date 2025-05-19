#include <math.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "board.h"
#include "engine.h"
#include "eval.h"
#include "movegen.h"
#include "table.h"
#include "utils.h" // includes <stdio.h>

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

volatile int STOP_SEARCH;
volatile int SEARCH_TIME;
volatile int SEARCHING;

U64 NUM_NODES;
U8 HIGHEST_DEPTH;

static pthread_t SEARCH_THREAD;
static Board *CURR_BOARD;
static bool UCI_DEBUG_ON = false;

// ~ debug ~
static Move MOVE_HISTORY[64];
static int MOVE_HISTORY_IDX;
// ~ debug ~

static void print_info(Board* board, U8 depth, double time) {
    TTEntry* entry = tt_probe(get_hash(board));
    int i = 0;
    if (!entry)
        return;

    int side_coeff = (CURR_BOARD->side_to_move * (-2) + 1);

    printf("info depth %d seldepth %d score ", depth, HIGHEST_DEPTH);
    if (abs(entry->score) > CHECKMATE_CP) {
        printf("mate %d", mate_score(entry));
    } else {
        printf("cp %d", entry->score * side_coeff);
    }

    TTEntry* d_entry = entry;
    Board* copy = copy_board(board);
    if (entry->depth > 0)
        printf(" pv");
    
    while (d_entry && i < entry->depth) {
        printf(" ");
        print_move(d_entry->best);

        make_move(copy, d_entry->best);
        d_entry = tt_probe(board_hash(copy));
        i++;
    }
    free(copy);

    if (NUM_NODES > 0) {
        printf(" nodes %lu", NUM_NODES);
    }

    if (time != 0) {
        printf(" nps %.0lf", (double)(NUM_NODES / time));
        printf(" time %.0lf", time * 1000);
    }

    printf("\n");
}

static void* search(void* arg) {
    Board *board = copy_board(CURR_BOARD);
    SearchParams params = *(SearchParams*)arg;
    SEARCH_TIME = params.movetime;
    TTEntry* entry;
    U8 curr_depth = 0;
    U64 hash = get_hash(board);
    Move best = 0, ponder = 0;
    clock_t start = clock(), end = clock();

    start_timer();
    do {
        curr_depth++;

        NUM_NODES = 0;
        start = clock();
        eval(board, curr_depth);
        end = clock();
        entry = tt_probe(hash);

        if (entry) {
            best = entry->best;
            make_move(board, best);
            entry = tt_probe(get_hash(board));
            if (entry)
                ponder = entry->best;
            unmake_move(board, best);
        }
        if (!STOP_SEARCH)
            print_info(board, curr_depth, (double)(end - start) / CLOCKS_PER_SEC);
        
    } while (curr_depth < params.depth && !STOP_SEARCH);
    STOP_SEARCH = 0;

    printf("bestmove ");
    print_move(best);
    if (ponder) {
        printf(" ponder ");
        print_move(ponder);
    }

    printf("\n");

    free_board(board);
    SEARCHING = 0;

    return NULL;
}

void engine_init() {
    // ~ debug ~
    memset(MOVE_HISTORY, 0, sizeof(Move) * 64);
    MOVE_HISTORY_IDX = 0;
    // ~ debug ~

    STOP_SEARCH = 0;
    SEARCHING = 0;
    CURR_BOARD = NULL;
    init_move_lookup_tables();
    init_zobrist();
    resize_engine_table(DEFAULT_TT_SIZE);
}

void engine_set_debug(bool mode) {
    UCI_DEBUG_ON = mode;
}

bool engine_is_debug() {
    return UCI_DEBUG_ON;
}

void engine_move(char* move_str) {
    if (!CURR_BOARD)
        return;

    Move move = move_from_str(CURR_BOARD, move_str);
    make_move(CURR_BOARD, move);
    MOVE_HISTORY[MOVE_HISTORY_IDX] = move;
    MOVE_HISTORY_IDX++;
}

void engine_unmove() {
    if (!CURR_BOARD || !MOVE_HISTORY_IDX)
        return;

    MOVE_HISTORY_IDX--;
    unmake_move(CURR_BOARD, MOVE_HISTORY[MOVE_HISTORY_IDX]);
}

void engine_quit() {
    if (CURR_BOARD)
        free_board(CURR_BOARD);
}

void resize_engine_table(int mb_size) {
    tt_set_size(mb_size);
}

int set_position(char* fen, char** moves) {
    if (CURR_BOARD)
        free(CURR_BOARD);

    if (fen == NULL)
        fen = START_FEN;

    CURR_BOARD = from_fen(fen);

    if (moves != NULL) {
        while (*moves) {
            Move move = move_from_str(CURR_BOARD, *moves);
            make_move(CURR_BOARD, move);
            moves++;
        }
    }

    return 0;
}

void print_engine() {
    if (!CURR_BOARD)
        return;

    printf("FEN: %s\n", to_fen(CURR_BOARD));
    if (UCI_DEBUG_ON) {
        printf("Internl Hash:  %lx\n", get_hash(CURR_BOARD));
        printf("External Hash: %lx\n", board_hash(CURR_BOARD));
        printf("Is Threefold:  %d\n", is_threefold(CURR_BOARD));
    }
    print_board(CURR_BOARD);

}

void go_perft(int depth) {
    if (!CURR_BOARD)
        return;

    print_perft(CURR_BOARD, depth);
}

void go_random() {
    if (!CURR_BOARD)
        return;

    printf("bestmove ");
    print_move(random_move(CURR_BOARD));
    printf("\n");
}

void start_search(SearchParams params) {
    if (!CURR_BOARD)
        return;

    SearchParams* ptr = malloc(sizeof(SearchParams));

    if (ptr != NULL) {
        *ptr = params;
    }

    SEARCHING = 1;
    pthread_create(&SEARCH_THREAD, NULL, search, ptr);
}


int stop_search() {
    if (!SEARCHING)
        return 0;

    STOP_SEARCH = 1;
    pthread_join(SEARCH_THREAD, NULL);

    return 1;
}
