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

static pthread_t SEARCH_THREAD;
static Board *CURR_BOARD;

static void print_pv(PrincipleVariation *pv, double time) {
    printf("info depth %d score ", pv->depth);
    if (pv->is_mate) {
        printf("mate %d", ((pv->depth + 1) / 2) * (CURR_BOARD->side_to_move * (-2) + 1)); // TODO remove -1
    } else {
        printf("cp %d", pv->score);
    }

    if (pv->depth > 0) {
        printf(" pv");

        for (int i = 0; i < pv->depth; i++) {
            printf(" ");
            print_move(pv->line[i]);
        }
    }

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
    PrincipleVariation pv;
    PrincipleVariation new_pv;
    SearchParams params = *(SearchParams*)arg;
    SEARCH_TIME = params.movetime;
    int curr_depth = 0;
    clock_t start, end;

    start_timer();
    do {
        curr_depth++;
        if (curr_depth > 1) {
            pv = new_pv;
            print_pv(&pv, (double)(end - start) / CLOCKS_PER_SEC);
        }

        NUM_NODES = 0;
        start = clock();
        new_pv = eval(board, curr_depth);
        end = clock();
        
    } while ((params.depth < 0 || (curr_depth < params.depth)) && !STOP_SEARCH);

    if (!STOP_SEARCH || curr_depth <= 1) {
        pv = new_pv;
        print_pv(&pv, (double)(end - start) / CLOCKS_PER_SEC);
    }
    STOP_SEARCH = 0;

    printf("bestmove ");
    print_move(pv.line[0]);
    if (pv.depth > 1) {
        printf(" ponder ");
        print_move(pv.line[1]);
    }
    printf("\n");

    free_board(board);
    SEARCHING = 0;

    return NULL;
}

void engine_init() {
    STOP_SEARCH = 0;
    SEARCHING = 0;
    CURR_BOARD = NULL;
    init_move_lookup_tables();
    init_zobrist();
    resize_engine_table(DEFAULT_TT_SIZE);
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

    printf("FEN:  %s\n", to_fen(CURR_BOARD));
    printf("Hash: %lx\n", board_hash(CURR_BOARD));
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
