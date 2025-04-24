#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "uci.h"
#include "board.h"
#include "utils.h" // includes <stdio.h>

#define IDENTIFY_STR "id name Menziesii\nid author Abraham Engebretson\n"

#define MAX_STR_SIZE 128

typedef enum {
    INITIAL_STATE
} UCIState;

static Board *G_BOARD;

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
    Move move;
    char* pt;
    int i;
    int state = 0;

    if (G_BOARD)
        free_board(G_BOARD);

    while (**input != '\n') {
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

static void go(char** input) {
    //char* pt;

    if (**input == '\n') {
        printf("bestmove ");
        print_move(random_move(G_BOARD));
        printf("\n");
    }

    while (**input != '\n') {
        if (has(input, "perft")) {
            if (G_BOARD)
                print_perft(G_BOARD, atoi(next_token(input)));

            break;
        } else {
            consume_token(input);
        }
    }
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
