#include <ctype.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include "board.h"
#include "eval.h"
#include "uci.h"
#include "utils.h" // includes <stdio.h>

#define IDENTIFY_STR "id name Menziesii\nid author Abraham Engebretson\n"

#define MAX_STR_SIZE 128

typedef enum {
    INF,
    DEPTH
} SearchType;

typedef enum {
    INITIAL_STATE
} UCIState;

static Board *G_BOARD;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static const int NUM_TEST_FENS = 4;
static char *TEST_FENS[] = {
    "k7/8/8/8/8/8/8/7K w - - 0 1", // X
    "k7/4R3/4PR2/5P2/8/8/8/7K w - - 0 1", // X
    "k7/8/6b1/8/5b2/4b3/8/7K b - - 0 1", // X
    "8/p2B2B1/p7/p7/p7/p7/pr6/k6K w - - 0 1" // X
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

static void print_pv(PrincipleVariation *pv) {
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
    int depth = *(int*)arg;
    int curr_depth = 0;

    do {
        curr_depth++;

        pv = eval(board, curr_depth);
        print_pv(&pv);
        
    } while (depth < 0 || (curr_depth < depth) /* TODO given stop command */);


    free(board);
    PrincipleVariation* heap_pv = malloc(sizeof(PrincipleVariation));
    *heap_pv = pv;
    return heap_pv;
}

static void go(char** input) {
    //char* pt;
    int depth = -1;

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
        } else if (has(input, "depth")) {
            depth = atoi(next_token(input));
        }
    }

    PrincipleVariation* pv;
    pthread_t search_thread;
    pthread_create(&search_thread, NULL, search, &depth);
    pthread_join(search_thread, (void**)&pv);
    printf("bestmove ");
    print_move(pv->line[0]);
    printf("\n");
    free(pv);
}

static void stop() {
    
}

int uci(void) {
    char *str = (char*)malloc(MAX_STR_SIZE * sizeof(char));
    G_BOARD = NULL;

    printf(IDENTIFY_STR);
    printf("uciok\n");
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


