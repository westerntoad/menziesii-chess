#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "board.h"
#include "eval.h"
#include "uci.h"
#include "utils.h" // includes <stdio.h>

#define IDENTIFY_STR "id name Menziesii\nid author Abraham Engebretson\n"

#define MAX_STR_SIZE 2<<15

volatile int STOP_SEARCH;
volatile int SEARCH_TIME;

U64 NUM_NODES;

static pthread_t SEARCH_THREAD;

static Board *G_BOARD;

static const int NUM_TEST_FENS = 5;
static char *TEST_FENS[] = {
    "k7/8/8/8/8/8/8/7K w - - 0 1", // X
    "k7/4R3/4PR2/5P2/8/8/8/7K w - - 0 1", // X
    "k7/8/6b1/8/5b2/4b3/8/7K b - - 0 1", // X
    "8/p2B2B1/p7/p7/p7/p7/pr6/k6K w - - 0 1", // X
    "r1bqkb1r/pppppppp/2n2n2/1N6/8/8/PPPPPPPP/R1BQKBNR w KQkq - 4 3"
};

static char* next_token(char** input) {
    char* pt = *input;

    while (!isspace(**input) && **input != '\n' && **input != '\0')
        (*input)++;

    if (**input == '\n') {
        (**input) = '\0';
        (*input)++;
        (**input) = '\n';
    } else {
        (**input) = '\0';
    }

    while (**input == '\0' || (isspace(**input) && **input != '\n'))
        (*input)++;

    return pt;
}

// advances pointer to the next word delimitted by whitespace
static void consume_token(char** input) {
    while (!isspace(**input) && **input != '\n' && **input != '\0')
        (*input)++;

    while (isspace(**input) && **input != '\n')
        (*input)++;
}

// takes input, and if input begins with word, return 1 & advance pointer until
// after word and all subsequent whitespace.
static int has(char** input, char* word) {
    int n = strlen(word);
    if ((strncmp(*input, word, n) == 0) && (isspace((*input)[n]))) {
        consume_token(input);
        return 1;
    }

    return 0;
}

static void print_pv(PrincipleVariation *pv, double time) {
    printf("info depth %d score ", pv->depth);
    if (pv->is_mate) {
        printf("mate %d", ((pv->depth + 1) / 2) * (G_BOARD->side_to_move * (-2) + 1)); // TODO remove -1
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

static void position(char** input) {
    Move move;
    char buff[64];
    char* pt;
    int i;
    int state = 0;

    if (G_BOARD)
        free_board(G_BOARD);

    while (**input != '\n') {
        for (i = 0; i < NUM_TEST_FENS && state == 0; i++) {
            snprintf(buff, 64, "%d", i+1);
            if (has(input, buff)) {
                G_BOARD = from_fen(TEST_FENS[i]);
                state = 1;
            }
        }

        if (has(input, "startpos") && state == 0) {
            G_BOARD = from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            state = 1;
        } else if (has(input, "fen")) {
            pt = *input;
            for (i = 0; i < 6; i++) {
                consume_token(input);
            }
            if (**input == '\n') {
                **input = '\0';
                G_BOARD = from_fen(pt);
                break;
            } else {
                *((*input) - 1) = '\0';
                G_BOARD = from_fen(pt);
            }

            state = 1;
        } else if (has(input, "moves") && state == 1) {
            state = 2;
        } else if (state == 2) {
            move = move_from_str(G_BOARD, next_token(input));
            make_move(G_BOARD, move);
        } else {
            consume_token(input);
        }
    }
}

static void* search(void* arg) {
    Board *board = copy_board(G_BOARD);
    PrincipleVariation pv;
    PrincipleVariation new_pv;
    int depth = *(int*)arg;
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
        
    } while ((depth < 0 || (curr_depth < depth)) && !STOP_SEARCH);

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

    free(board);
    return NULL;
}

static void go(char** input) {
    //char* pt;
    int depth = -1;
    int wtime=0, btime=0, winc=0, binc=0;
    bool go_infinite = false;
    SEARCH_TIME = 0;

    while (**input != '\n') {
        if (has(input, "perft")) {
            if (G_BOARD)
                print_perft(G_BOARD, atoi(next_token(input)));

            return;
        } else if (has(input, "random")) {
            if (G_BOARD) {
                printf("bestmove ");
                print_move(random_move(G_BOARD));
                printf("\n");
            }

            return;
        } else if (has(input, "movetime")) {
            SEARCH_TIME = atoi(next_token(input));
        } else if (has(input, "depth")) {
            depth = atoi(next_token(input));
        } else if (has(input, "wtime")) {
            wtime = atoi(next_token(input));
        } else if (has(input, "btime")) {
            btime = atoi(next_token(input));
        } else if (has(input, "winc")) {
            winc = atoi(next_token(input));
        } else if (has(input, "binc")) {
            binc = atoi(next_token(input));
        } else if (has(input, "infinite")) {
            go_infinite = true;
        } else {
            consume_token(input);
        }
    }

    if (SEARCH_TIME == 0 && !go_infinite) {
        if (G_BOARD->side_to_move == WHITE) {
            SEARCH_TIME = MIN(1000, winc) + MIN(2000, wtime / 4);
        } else {
            SEARCH_TIME = MIN(1000, binc) + MIN(2000, btime / 4);
        }
    }
    pthread_create(&SEARCH_THREAD, NULL, search, &depth);
}

static void stop() {
    STOP_SEARCH = 1;
    pthread_join(SEARCH_THREAD, NULL);
}

int uci(void) {
    STOP_SEARCH = 0;
    char *str = (char*)malloc(MAX_STR_SIZE * sizeof(char));
    G_BOARD = NULL;

    printf(IDENTIFY_STR);
    printf("\nuciok\n");
    while (1) {
        if (fgets(str, MAX_STR_SIZE, stdin) == NULL) {
            return -1;
        }

        while (*str != '\n' && *str != '\0') {
            if (has(&str, "isready")) {
                printf("readyok\n");
                break;
            } else if (has(&str, "position")) {
                position(&str);
                break;
            } else if (has(&str, "go")) {
                go(&str);
                break;
            } else if (has(&str, "stop")) {
                stop();
                break;
            } else if (has(&str, "quit")) {
                //free(str); // TODO fix
                if (G_BOARD)
                    free_board(G_BOARD);

                return 0;
            } else if (has(&str, "d")) {
                if (G_BOARD)
                    print_board(G_BOARD);

                break;
            } else {
                consume_token(&str);
            }
        }
    }
    
    return 0;
}


