#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>  // used for unicode printing
#include "board.h"
#include "movegen.h"
#include "utils.h" // includes <stdio.h>

#define INITIAL_MOVE_CAPACITY 100
#define HALF_MOVE_MASK 0x1ffff

static U64 danger_squares(Board *board) {
    U64 friendly = board->colors[board->side_to_move];
    U64 enemy = board->colors[board->side_to_move ^ 1];
    U64 friendly_k = board->pieces[KING_IDX] & friendly;
    U64 enemy_n = board->pieces[KNIGHT_IDX] & friendly;
    U64 enemy_b = board->pieces[BISHOP_IDX] & friendly;
    U64 enemy_r = board->pieces[ROOK_IDX] & friendly;
    U64 enemy_q = board->pieces[QUEEN_IDX] & friendly;
    U64 enemy_k = board->pieces[KING_IDX] & friendly;
    U64 danger_squares = 0ULL;
    while(enemy_n)
        danger_squares |= n_moves(pop_lsb(&enemy_n));
    while(enemy_b)
        danger_squares |= b_moves(pop_lsb(&enemy_b), enemy | (friendly & !friendly_k)) & ~friendly;
    while(enemy_r)
        danger_squares |= r_moves(pop_lsb(&enemy_r), enemy | (friendly & !friendly_k)) & ~friendly;
    while(enemy_q)
        danger_squares |= q_moves(pop_lsb(&enemy_q), enemy | (friendly & !friendly_k)) & ~friendly;
    while(enemy_k)
        danger_squares |= k_moves(pop_lsb(&enemy_k));

    return danger_squares;

}

void make_move(Board *board, Move move) {
    U64 from = 1ULL << get_from(move);
    U64 to = 1ULL << get_to(move);
    U64 aux1, aux2;
    bool curr_color = board->side_to_move;
    StateFlags next_state = board->state_stack[board->ply] + 1;
    int i, j;

    board->ply++;
    board->side_to_move ^= 1;

    if (((move >> 12) & 0b1110) == 0b10) { // check if castle
        from = 0x8100000000000081ULL; // rook origin
        to   = 0x2800000000000028ULL; // rook target
        aux1 = 0x1000000000000010ULL; // king origin
        aux2 = 0x4400000000000044ULL; // king target

        if ((move>>12) & 1) { // if is long castle
            from &= 0x0100000000000001ULL;
            to   &= 0x0800000000000008ULL;
            aux2 &= 0x0400000000000004ULL;
        } else {
            from &= 0x8000000000000080ULL;
            to   &= 0x2000000000000020ULL;
            aux2 &= 0x4000000000000040ULL;
        }

        if (curr_color) { // if black
            from &= 0xff00000000000000ULL;
            to   &= 0xff00000000000000ULL;
            aux1 &= 0xff00000000000000ULL;
            aux2 &= 0xff00000000000000ULL;
            next_state &= 0xe7ffffff; // clear black castling-bits
        } else {
            from &= 0x00000000000000ffULL;
            to   &= 0x00000000000000ffULL;
            aux1 &= 0x00000000000000ffULL;
            aux2 &= 0x00000000000000ffULL;
            next_state &= 0x9fffffff; // clear white castling-bits
        }

        board->pieces[ROOK_IDX] &= ~from; // remove old pieces
        board->pieces[KING_IDX] &= ~aux1;
        board->colors[curr_color] &= ~(from | aux1);
        board->pieces[ROOK_IDX] |= to; // add new pieces
        board->pieces[KING_IDX] |= aux2;
        board->colors[curr_color] |= to | aux2;

        goto end; // gosh
    }

    
    for (i = 0; i < NUM_PIECES; i++) {
        if (board->pieces[i] & from)
            break;
    }
    
    if (move >> 12 == 5) { // check if ep capture
        aux1 = curr_color ? sout_one(to) : nort_one(to); // ep-captured pawn
        
        board->pieces[PAWN_IDX] &= aux1;
        board->colors[curr_color ^ 1] &= aux1;
        next_state &= 0xfffe0000; // clear half-move clock
    } else if (move & 0x4000) { // check if capture
        for (j = 0; j < NUM_PIECES; j++) {
            if (board->pieces[j] & to)
                break;
        }

        board->pieces[j] &= ~to;
        board->colors[curr_color ^ 1] &= ~to;

        next_state |= j << 17; // store captured piece-index
        next_state &= 0xfffe0000; // clear half-move clock
    }


    if (move >> 12 == 1) { // check if double pawn push
        next_state |= 0x04000000; // set ep target exist
        if (curr_color) { // if black
            next_state |= LOG2(nort_one(to)) << 20;
        } else {
            next_state |= LOG2(sout_one(to)) << 20;
        }
    } else {
        // if not double pawn push, clear ep target mask
        next_state &= 0xfbffffff;
    }

    // additional checks for prohibiting castling
    if (from & 0x1000000000000010ULL) { // if origin is a king
        next_state &= curr_color ? 0xe7ffffff : 0x9fffffff;
    } else if (from & 0x8000000000000080) { // if origin is kingside rook
        next_state &= curr_color ? 0xefffffff : 0xbfffffff;
    } else if (from & 0x0100000000000001) { // if origin is queenside rook
        next_state &= curr_color ? 0xf7ffffff : 0xdfffffff;
    }

    board->pieces[i] &= ~from;
    board->colors[curr_color] &= ~from;
    board->pieces[i] |= to;
    board->colors[curr_color] |= to;

end:
    board->state_stack[board->ply] = next_state; // TODO check if ply >= capacity
}

void unmake_move(Board *board, Move move) {
    U64 from = 1ULL << get_from(move);
    U64 to = 1ULL << get_to(move);
    U64 aux1, aux2;
    board->side_to_move ^= 1;
    bool curr_color = board->side_to_move;
    int i, captured_piece_idx;
    StateFlags next_state = board->state_stack[board->ply--];

    if (((move >> 12) & 0b1110) == 0b10) { // check if castle
        from = 0x8100000000000081ULL; // rook origin
        to   = 0x2800000000000028ULL; // rook target
        aux1 = 0x1000000000000010ULL; // king origin
        aux2 = 0x4400000000000044ULL; // king target

        if ((move>>12) & 1) { // if is long castle
            from &= 0x0100000000000001ULL;
            to   &= 0x0800000000000008ULL;
            aux2 &= 0x0400000000000004ULL;
        } else {
            from &= 0x8000000000000080ULL;
            to   &= 0x2000000000000020ULL;
            aux2 &= 0x4000000000000040ULL;
        }

        if (curr_color) { // if black
            from &= 0xff00000000000000ULL;
            to   &= 0xff00000000000000ULL;
            aux1 &= 0xff00000000000000ULL;
            aux2 &= 0xff00000000000000ULL;
        } else {
            from &= 0x00000000000000ffULL;
            to   &= 0x00000000000000ffULL;
            aux1 &= 0x00000000000000ffULL;
            aux2 &= 0x00000000000000ffULL;
        }

        board->pieces[ROOK_IDX] &= ~to; // remove old pieces
        board->pieces[KING_IDX] &= ~aux2;
        board->colors[curr_color] &= ~(to | aux2);
        board->pieces[ROOK_IDX] |= from; // add new pieces
        board->pieces[KING_IDX] |= aux1;
        board->colors[curr_color] |= from | aux1;

        return;
    }


    for (i = 0; i < NUM_PIECES; i++) {
        if (board->pieces[i] & to) {
            break;
        }
    }

    if (move >> 12 == 5) { // check if ep capture
        aux1 = curr_color ? nort_one(to) : sout_one(to); // ep-captured pawn
        
        board->pieces[PAWN_IDX] |= aux1;
        board->colors[curr_color ^ 1] |= aux1;
    } else if (move & 0x4000) { // check if capture
        captured_piece_idx = (next_state >> 17) & 0x07; // restore captured piece from state_stack

        board->pieces[captured_piece_idx] |= to;
        board->colors[curr_color ^ 1] |= to;
    }

    board->pieces[i] &= ~to;
    board->colors[curr_color] &= ~to;
    board->pieces[i] |= from;
    board->colors[curr_color] |= from;
}

int legal_moves(Board *board) {
    wprintf(L"\n");
    print_bb(danger_squares(board));
    return 0;
}

bool can_castle(Board *board, bool color, bool side) {
    return TEST_BIT(board->state_stack[board->ply], 27 + (side^1) + (color^1)*2);
}

int half_moves(Board *board) {
    return board->state_stack[board->ply] & HALF_MOVE_MASK;
}

Board* from_fen(char* fen) {
    Board *board = (Board*)malloc(sizeof(Board));
    memset(board, 0, sizeof(Board));
    board->stack_capacity = INITIAL_MOVE_CAPACITY;
    board->state_stack = (StateFlags *)calloc(board->stack_capacity, sizeof(StateFlags));
    board->state_stack[0] = 1 << 31;
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

    i++;

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
    StateFlags state = board->state_stack[board->ply];
    U64 bb;
    int i, j, k, c, color, piece;
    char castle_rights[5];
    wprintf(L"          %c to move\n", board->side_to_move ? 'b' : 'w');
    for (i = 7; i >= 0; i--) {
        wprintf(L"     %d ", i + 1);
        for (j = 0; j < 8; j++) {
            c = i % 2 != j % 2 ? '.' : ',';
            for (k = 0; k < NUM_PIECES * NUM_COLORS; k++) {
                piece = k % NUM_PIECES;
                color = k / NUM_PIECES;
                bb = board->pieces[piece] & board->colors[color];
                
                if ((bb >> (j+i*8)) & 1ULL) {
                    c = 0x2654 + (NUM_PIECES - piece - 1) + (color ^ 1) * NUM_PIECES;
                    break;
                }
            }
            wprintf(L"%lc ", c);
        }
        wprintf(L"\n");
    }
    wprintf(L"       ");
    for (i = 0; i < 8; i++) {
        wprintf(L"%c ", 0x61 + i);
    }
    wprintf(L"\n     ply %13d", board->ply);
    i = 0;
    if (can_castle(board, WHITE, KINGSIDE)) {
        castle_rights[i] = 'K';
        i++;
    }
    if (can_castle(board, WHITE, QUEENSIDE)) {
        castle_rights[i] = 'Q';
        i++;
    }
    if (can_castle(board, BLACK, KINGSIDE)) {
        castle_rights[i] = 'k';
        i++;
    }
    if (can_castle(board, BLACK, QUEENSIDE)) {
        castle_rights[i] = 'q';
        i++;
    }
    if (i == 0) {
        castle_rights[i] = '-';
        i++;
    }
    castle_rights[i] = '\0';
    wprintf(L"\n     castle %10s", castle_rights);
    wprintf(L"\n     half_move %7d", half_moves(board));
    wprintf(L"\n     ep_target %5s", "");
    if (TEST_BIT(state, 26)) {
        print_sq((state >> 20) & 0x3f);
    } else {
        wprintf(L" -");
    }
    wprintf(L"\n");
}
