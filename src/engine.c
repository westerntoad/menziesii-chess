#include <math.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "board.h"
#include "engine.h"
#include "eval.h"
#include "movegen.h"
#include "utils.h" // includes <stdio.h>

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
    PrincipleVariation pv;
    PrincipleVariation new_pv;
    SearchParams params = *(SearchParams*)arg;
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
        new_pv = eval(CURR_BOARD, curr_depth);
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

    SEARCHING = 0;
    return NULL;
}

void engine_init() {
    STOP_SEARCH = 0;
    init_move_lookup_tables();
}

void start_search(Board *board, SearchParams params) {
    SEARCHING = 1;
    if (CURR_BOARD) {
        free(CURR_BOARD);
    }
    CURR_BOARD = copy_board(board);
    pthread_create(&SEARCH_THREAD, NULL, search, &params);
}

int stop_search() {
    STOP_SEARCH = 1;
    pthread_join(SEARCH_THREAD, NULL);

    return SEARCHING;
}
