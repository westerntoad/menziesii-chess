#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "engine.h"
#include "eval.h"
#include "uci.h"
#include "utils.h" // includes <stdio.h>

#define IDENTIFY_NAME "Menziesii"
#define IDENTIFY_AUTHOR "Abraham Engebretson"
#define MAX_STR_SIZE 2<<15

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

static void position(char** input) {
    char* fen = NULL;
    char moves[512][5] = { 0 };
    int i = 0;
    int state = 0;

    while (**input != '\n') {
        if (has(input, "startpos") && state == 0) {
            state = 1;
        } else if (has(input, "fen")) {
            fen = *input;
            for (i = 0; i < 6; i++) {
                consume_token(input);
            }
            if (**input == '\n') {
                **input = '\0';
                state = 1;
                break;
            } else {
                *((*input) - 1) = '\0';
            }

            i = 0;
            state = 1;
        } else if (has(input, "moves") && state == 1) {
            state = 2;
        } else if (state == 2) {
            strncpy(moves[i], next_token(input), 4);
            i++;
        } else {
            consume_token(input);
        }
    }

    strcpy(moves[i], "");

    if (state > 0) {
        set_position(fen, moves);
    }
}

static void go(char** input) {
    SearchParams params = PARAMS_DEFAULT;

    while (**input != '\n') {
        if (has(input, "perft")) {
            go_perft(atoi(next_token(input)));

            return;
        } else if (has(input, "random")) {
            go_random();

            return;
        } else if (has(input, "movetime")) {
            params.movetime = atoi(next_token(input));
        } else if (has(input, "depth")) {
            params.depth = atoi(next_token(input));
        } else if (has(input, "wtime")) {
            params.wtime = atoi(next_token(input));
        } else if (has(input, "btime")) {
            params.btime = atoi(next_token(input));
        } else if (has(input, "winc")) {
            params.winc = atoi(next_token(input));
        } else if (has(input, "binc")) {
            params.binc = atoi(next_token(input));
        } else if (has(input, "infinite")) {
            params.infinite = 1;
        } else { // TODO searchmoves
            consume_token(input);
        }
    }

    start_search(params);
}

static void stop() {
    stop_search();
}

int uci(void) {
    engine_init();
    char *str = (char*)malloc(MAX_STR_SIZE * sizeof(char));

    printf("id name %s dev-%d-%s\nid author %s\n", IDENTIFY_NAME, BUILD_DATE, GIT_HASH, IDENTIFY_AUTHOR);
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
                engine_quit();

                return 0;
            } else if (has(&str, "d")) {
                print_engine();

                break;
            } else {
                consume_token(&str);
            }
        }
    }
    
    return 0;
}


