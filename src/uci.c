#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "uci.h"
#include "board.h"
#include "utils.h" // includes <stdio.h>

#define IDENTIFY_STR "id name Menziesii\nid author Abraham Engebretson\n"

#define MAX_STR_SIZE 64

typedef enum {
    INITIAL_STATE
} UCIState;

static Board *G_BOARD;

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
    char* pt;
    int i;

    while (**input != '\n') {
        if (has(input, "startpos")) {
            G_BOARD = from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        } else if (has(input, "fen")) {
            pt = *input;
            for (i = 0; i < 6; i++) {
                consume_token(input);
            }
            if (**input == '\n') {
                **input = '\0';
                printf("%s\n", pt);
                printf("\n\n\n\n\nYEAH\n\n\n\n\n");
                G_BOARD = from_fen(pt);
                break;
            } else {
                *((*input) - 1) = '\0';
                G_BOARD = from_fen(pt);
            }
        } else {
            consume_token(input);
        }
    }

    /*if (G_BOARD)
        print_board(G_BOARD);*/
}

static void go(char** input) {
    printf("%s", *input);
}

int uci(void) {
    char *str = (char*)malloc(MAX_STR_SIZE * sizeof(char));
    G_BOARD = NULL;

    printf(IDENTIFY_STR);
    printf("uciok\n");
    while (1) {
        fgets(str, MAX_STR_SIZE, stdin);

        while (*str != '\n') {
            if (has(&str, "isready")) {
                printf("readyok\n");
                break;
            } else if (has(&str, "position")) {
                position(&str);
                break;
            } else if (has(&str, "go")) {
                go(&str);
                break;
            } else  {
                consume_token(&str);
            }
        }
    }
    
    return 0;
}
