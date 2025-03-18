#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "board.h"
#include "utils.h" // includes <stdio.h>

#define INITIAL_MOVE_CAPACITY 100

Board* from_fen(char* fen) {
    Board *board = (Board*)malloc(sizeof(Board));
    memset(board, 0, sizeof(Board));
    board->move_capacity = INITIAL_MOVE_CAPACITY;
    board->state_stack = (StateFlags *)calloc(board->move_capacity, sizeof(StateFlags));
    U64 bb;
    int i, j, color_idx = 0, piece_idx = 0;
    char c;

    for (i = 0, j = 0; fen[i] != ' '; i++, j++) {
        c = fen[i];
        if (isdigit(c)) {
            j += c - 49;
        } else if (c == '/') {
            j--;
        } else {
            bb = 1ULL << (((63 - j) / 8) * 8 + (j % 8));

            color_idx = isupper(c) ? WHITE : BLACK;

            switch (c) {
                case 'p': case 'P':
                    piece_idx = PAWN_IDX;
                    break;
                case 'n': case 'N':
                    piece_idx = KNIGHT_IDX;
                    break;
                case 'b': case 'B':
                    piece_idx = BISHOP_IDX;
                    break;
                case 'r': case 'R':
                    piece_idx = ROOK_IDX;
                    break;
                case 'q': case 'Q':
                    piece_idx = QUEEN_IDX;
                    break;
                case 'k': case 'K':
                    piece_idx = KING_IDX;
                    break;
                default:
                    debug_assert(false, "FEN character placement must be one of { p, n, b, r, q, k } uppercase/lowercase.");
            }

            board->colors[color_idx] |= bb;
            board->pieces[piece_idx] |= bb;
        }
    }
    debug_assert(j == 63, "FEN translation for piece placement must equal 64");

    i++;

    board->side_to_move = fen[i++] == 'w' ? WHITE : BLACK;

    if (fen[i] != '-') {
        for (; fen[i] != ' '; i++) {
            c = fen[i];
            switch (c) {
                case 'K':
                    board->state_stack[0] = SET_BIT(board->state_stack[0], 30);
                    break;
                case 'Q':
                    board->state_stack[0] = SET_BIT(board->state_stack[0], 29);
                    break;
                case 'k':
                    board->state_stack[0] = SET_BIT(board->state_stack[0], 28);
                    break;
                case 'q':
                    board->state_stack[0] = SET_BIT(board->state_stack[0], 27);
                    break;
                default:
                    debug_assert(false, "FEN castling rights must be one of { k, q } uppercase/lowercase.");
            }
        }
    } else {
        i++; 
    }

    i++; 

    for (; fen[i] != ' '; i++); // TODO en passant
    i++;
    for (; fen[i] != ' '; i++); // TODO half-move clock
    i++;
    for (; fen[i] != ' '; i++); // TODO full-move num

    return board;
}

void print_board(Board *board) {
    printf("WHITE:\n");
    print_bb(board->colors[WHITE]);
    printf("BLACK:\n");
    print_bb(board->colors[BLACK]);

    printf("\nPAWN:\n");
    print_bb(board->pieces[PAWN_IDX]);
    printf("KNIGHT:\n");
    print_bb(board->pieces[KNIGHT_IDX]);
    printf("BISHOP:\n");
    print_bb(board->pieces[BISHOP_IDX]);
    printf("ROOK:\n");
    print_bb(board->pieces[ROOK_IDX]);
    printf("QUEEN:\n");
    print_bb(board->pieces[QUEEN_IDX]);
    printf("KING:\n");
    print_bb(board->pieces[KING_IDX]);
}
