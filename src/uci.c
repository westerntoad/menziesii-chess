#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "engine.h"
#include "uci.h"
#include "utils.h" // includes <stdio.h>

#define IDENTIFY_NAME "Menziesii"
#define IDENTIFY_AUTHOR "Abraham Engebretson"
#define INITIAL_READ_SIZE 2<<5
#define INITIAL_MOVE_LENGTH 2<<5

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

static char** read_moves(char** input) {
    char** moves = malloc(sizeof(char*) * INITIAL_MOVE_LENGTH);
    int i = 0;
    int capacity = INITIAL_MOVE_LENGTH;

    while (**input != '\n') {
        if (i >= capacity) {
            capacity *= 2;
            moves = realloc(moves, sizeof(char*) * capacity);
            if (moves == NULL) {
                fprintf(stderr, "Error allocating input string of size %d.\nExiting...", capacity);
                free(moves);
                exit(EXIT_FAILURE);
            }
        }

        moves[i] = *input;
        next_token(input);
        i++;
    }
    moves[i] = NULL;
    
    return moves;
}

static void position(char** input) {
    char* fen = NULL;
    char **moves = NULL;
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
            moves = read_moves(input);
            break;
        } else {
            consume_token(input);
        }
    }

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

static void setoption(char** input) {
    while (**input != '\n') {
        if (has(input, "Hash")) {
            if (has(input, "value")) {
                resize_engine_table(atoi(next_token(input)));
            }
            return;
        } else {
            consume_token(input);
        }
    }
}


static void stop() {
    stop_search();
}

static char* read() {
    char *read_str = malloc(sizeof(char) * INITIAL_READ_SIZE);
    int capacity = INITIAL_READ_SIZE;
    int i = 0;
    int curr = 0;

    while (curr != '\n') {
        if (i >= capacity) {
            capacity *= 2;
            read_str = realloc(read_str, sizeof(char) * capacity);
            if (read_str == NULL) {
                fprintf(stderr, "Error allocating input string of size %d.\nExiting...", capacity);
                free(read_str);
                exit(EXIT_FAILURE);
            }
        }

        curr = fgetc(stdin);
        read_str[i] = curr;

        i++;
    }
    return read_str;
}

int uci(void) {
    engine_init();
    char* read_str = read();
    char* ptr;

    while (1) { // read by line
        ptr = read_str;
        while (*ptr != '\n' && *ptr != '\0') { // read by token
            if (has(&ptr, "uci")) {
                printf("id name %s dev-%d-%s\nid author %s\nuciok\n", IDENTIFY_NAME, COMMIT_DATE, GIT_HASH, IDENTIFY_AUTHOR);
                printf("option name Hash type spin default %d min 1 max 65536\n", DEFAULT_TT_SIZE);
            } else if (has(&ptr, "isready")) {
                printf("readyok\n");
                break;
            } else if (has(&ptr, "position")) {
                position(&ptr);
                break;
            } else if (has(&ptr, "go")) {
                go(&ptr);
                break;
            } else if (has(&ptr, "setoption")) {
                if (has(&ptr, "name")) {
                    setoption(&ptr);
                }
            } else if (has(&ptr, "stop")) {
                stop();
                break;
            } else if (has(&ptr, "d")) {
                print_engine();

                break;
            } else if (has(&ptr, "debug")) {
                if (has(&ptr, "on")) {
                    engine_set_debug(true);
                    break;
                } else if (has(&ptr, "off")) {
                    engine_set_debug(false);
                    break;
                }
            } else if (has(&ptr, "move")) {
                char* move = ptr;
                next_token(&ptr);
                engine_move(move);
                break;
            } else if (has(&ptr, "unmove")) {
                engine_unmove();
                break;
            } else if (has(&ptr, "quit")) {
                engine_quit();

                free(read_str);
                return 0;
            } else {
                consume_token(&ptr);
            }
        }

        free(read_str);
        read_str = read();
    }
}


