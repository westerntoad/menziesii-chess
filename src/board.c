#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>  // used for unicode printing
#include "board.h"
#include "movegen.h"
#include "table.h"
#include "utils.h" // includes <stdio.h>

#define INITIAL_MOVE_CAPACITY 128
#define HALF_MOVE_MASK 0x1ffff

Board* copy_board(Board *board) { // DEBUG
    int i;
    Board *copy = (Board*)malloc(sizeof(Board));
    memset(copy, 0, sizeof(Board));
    copy->stack_capacity = board->stack_capacity;
    copy->state_stack = (StateFlags *)malloc(copy->stack_capacity * sizeof(StateFlags));

    for (i = 0; i < NUM_COLORS; i++)
        copy->colors[i] = board->colors[i];

    for (i = 0; i < NUM_PIECES; i++)
        copy->pieces[i] = board->pieces[i];

    for (i = 0; i <= board->ply; i++)
        copy->state_stack[i] = board->state_stack[i];

    copy->ply = board->ply;
    copy->side_to_move = board->side_to_move;
    copy->ply_offset = board->ply_offset;
    copy->hash = board->hash;

    return copy;
}

#if DEBUG
static bool boards_equal(Board *b1, Board *b2) { // DEBUG
    int i;

    if (b1->ply != b2->ply)
        return false;

    for (i = 0; i < NUM_COLORS; i++) {
        if (b1->colors[i] != b2->colors[i])
            return false;
    }

    for (i = 0; i < NUM_PIECES; i++) {
        if (b1->pieces[i] != b2->pieces[i])
            return false;
    }

    for (i = 0; i <= b1->ply; i++) {
        if (b1->state_stack[i] != b2->state_stack[i])
            return false;
    }

    return true;
}
#endif

static U64 danger_squares(Board *board) {
    U64 aux1, aux2;
    U64 friendly = board->colors[board->side_to_move];
    U64 enemy = board->colors[board->side_to_move ^ 1];
    U64 friendly_k = board->pieces[KING_IDX] & friendly;
    U64 enemy_p = board->pieces[PAWN_IDX] & enemy;
    U64 enemy_n = board->pieces[KNIGHT_IDX] & enemy;
    U64 enemy_b = board->pieces[BISHOP_IDX] & enemy;
    U64 enemy_r = board->pieces[ROOK_IDX] & enemy;
    U64 enemy_q = board->pieces[QUEEN_IDX] & enemy;
    U64 enemy_k = board->pieces[KING_IDX] & enemy;
    U64 blockers = enemy | (friendly & ~friendly_k);
    U64 danger_squares = 0ULL;
    while (enemy_p) {
        aux1 = pop_lsb(&enemy_p);
        aux2 = east_one(aux1) | west_one(aux1);
        danger_squares |= board->side_to_move ? nort_one(aux2) : sout_one(aux2);
    }
    while(enemy_n)
        danger_squares |= n_moves(pop_lsb(&enemy_n));
    while(enemy_b)
        danger_squares |= b_moves(pop_lsb(&enemy_b), blockers);
    while(enemy_r)
        danger_squares |= r_moves(pop_lsb(&enemy_r), blockers);
    while(enemy_q)
        danger_squares |= q_moves(pop_lsb(&enemy_q), blockers);
    while(enemy_k)
        danger_squares |= k_moves(pop_lsb(&enemy_k));

    return danger_squares;
}

static U64 get_checkers(Board *board, bool color) {
    U64 friendly = board->colors[color];
    U64 enemy = board->colors[color^1];
    U64 checkers = 0ULL;
    U64 king = board->pieces[KING_IDX] & friendly;
    U64 aux;

    aux = east_one(king) | west_one(king);
    aux = color ? sout_one(aux) : nort_one(aux);
    checkers |= aux & board->pieces[PAWN_IDX] & enemy;
    checkers |= n_moves(king) & board->pieces[KNIGHT_IDX] & enemy;
    aux = enemy | friendly;
    checkers |= b_moves(king, aux) & (board->pieces[BISHOP_IDX] | board->pieces[QUEEN_IDX]) & enemy;
    checkers |= r_moves(king, aux) & (board->pieces[ROOK_IDX  ] | board->pieces[QUEEN_IDX]) & enemy;

    return checkers;
}

static U64 ray_between(U64 from, U64 to) {
    const int dx[8] = { 0,  1,  0, -1,  1, -1,  1, -1 };
    const int dy[8] = { 1,  0, -1,  0,  1,  1, -1, -1 };
    U64 ray, aux;
    int i;

    for (i = 0; i < 8; i++) {
        ray = 0;
        aux = from;
        while (aux) {
            aux = delta_one(aux, dx[i], dy[i]);
            if (aux & to)
                return ray;

            ray |= aux;
        }
    }

    return 0;
}

static U64 get_pins(Board *board, bool color) {
    const int dx[8] = { 1, -1,  1, -1,  0,  1,  0, -1 };
    const int dy[8] = { 1,  1, -1, -1,  1,  0, -1,  0 };
    U64 friendly = board->colors[color];
    U64 enemy = board->colors[color^1];
    U64 king = board->pieces[KING_IDX] & friendly;
    U64 aux1, aux2, aux3;
    U64 enem;
    U64 opp;
    U64 pins = 0ULL;
    int i, j;

    for (i = 0; i < 2; i++) {
        aux1 = (board->pieces[BISHOP_IDX + i] | board->pieces[QUEEN_IDX]) & enemy;
        while (aux1) {
            aux2 = pop_lsb(&aux1);
            for (j = i*4; j < (i+1)*4; j++) {
                enem = 0ULL;
                opp = 0ULL;
                aux3 = aux2;
                do {
                    aux3 = delta_one(aux3, dx[j], dy[j]);
                    enem |= aux3;
                } while (aux3 & ~friendly & ~enemy);

                aux3 = king;
                do {
                    aux3 = delta_one(aux3, -dx[j], -dy[j]);
                    opp |= aux3;
                } while (aux3 & ~friendly & ~enemy);

                pins |= opp & enem & friendly;
            }
        }    
    }

    return pins;
}

static inline U64 ep_target(Board *board) {
    StateFlags state = board->state_stack[board->ply];
    return state & 0x04000000 ? 1ULL << ((state >> 20) & 0x3f) : 0ULL;
}

static inline bool can_castle(Board *board, bool color, bool side) {
    return TEST_BIT(board->state_stack[board->ply], 27 + (side^1) + (color^1)*2);
}

static int half_moves(Board *board) {
    return board->state_stack[board->ply] & HALF_MOVE_MASK;
}

void make_move(Board *board, Move move) {
    U64 from = 1ULL << get_from(move);
    U64 to = 1ULL << get_to(move);
    U64 aux1, aux2;
    Sq sq;
    bool curr_color = board->side_to_move;
    StateFlags next_state = board->state_stack[board->ply];
    StateFlags prev_state = next_state;
    int i, j;
    
    if (prev_state & 0x04000000) {
        //printf("REMOVING OLD EP  %d\n", ((prev_state >> 20) & 0x3f) % 8); // DEBUG
        // update hash to remove previous ep target
        board->hash ^= ZOBRIST_EP[((prev_state >> 20) & 0x3f) % 8];
    }

    next_state &= 0xf801ffff; // clear previous captured piece & ep_target
    next_state++;

    board->ply++;
    board->side_to_move ^= 1;
    board->hash ^= ZOBRIST_BLACK; // update hash

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

        board->hash ^= ZOBRIST_PIECE_SQ[ROOK_IDX][curr_color][LOG2(from)]; // update hash
        board->hash ^= ZOBRIST_PIECE_SQ[KING_IDX][curr_color][LOG2(aux1)];
        board->hash ^= ZOBRIST_PIECE_SQ[ROOK_IDX][curr_color][LOG2(to)];
        board->hash ^= ZOBRIST_PIECE_SQ[KING_IDX][curr_color][LOG2(aux2)];

        goto end; // gosh
    }

    
    for (i = 0; i < NUM_PIECES; i++) {
        if (board->pieces[i] & from)
            break;
    }

    if (i == PAWN_IDX)
        next_state &= 0xfffe0000; // clear half-move clock
    
    if (move >> 12 == 5) { // check if ep capture
        aux1 = curr_color ? nort_one(to) : sout_one(to); // ep-captured pawn
        
        board->pieces[PAWN_IDX] &= ~aux1; // clear ep_captured pawn
        board->colors[curr_color ^ 1] &= ~aux1;

        board->hash ^= ZOBRIST_PIECE_SQ[PAWN_IDX][curr_color ^ 1][LOG2(aux1)]; // update hash
    } else if (move & 0x4000) { // check if capture
        for (j = 0; j < NUM_PIECES; j++) {
            if (board->pieces[j] & to)
                break;
        }

        board->pieces[j] &= ~to; // clear captured piece
        board->colors[curr_color ^ 1] &= ~to;

        board->hash ^= ZOBRIST_PIECE_SQ[j][curr_color ^ 1][LOG2(to)]; // update hash

        next_state |= j << 17; // store captured piece-index
        next_state &= 0xfffe0000; // clear half-move clock
    }


    if (move >> 12 == 1) { // check if double pawn push
        next_state |= 0x04000000; // set ep target exist
        if (curr_color) { // if black
            sq = LOG2(nort_one(to));
        } else {
            sq = LOG2(sout_one(to));
        }
        next_state |= sq << 20;
        board->hash ^= ZOBRIST_EP[sq%8]; // update hash
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

    board->hash ^= ZOBRIST_PIECE_SQ[i][curr_color][LOG2(from)]; // update hash

    if (move & 0x8000) { // check if promotion
        j = (move >> 12) & 3;
        if (j == 0)
            i = KNIGHT_IDX;
        else if (j == 1)
            i = BISHOP_IDX;
        else if (j == 2)
            i = ROOK_IDX;
        else
            i = QUEEN_IDX;
    }

    board->pieces[i] |= to;
    board->colors[curr_color] |= to;
    board->hash ^= ZOBRIST_PIECE_SQ[i][curr_color][LOG2(to)]; // update hash

end:
    if (board->ply >= board->stack_capacity) {
        board->stack_capacity *= 2;
        board->state_stack = realloc(board->state_stack, sizeof(StateFlags) * board->stack_capacity);
        if (board->state_stack == NULL) {
            fprintf(stderr, "Error allocating new state stack of size %d.\nExiting...", board->stack_capacity);
            free(board);
            exit(EXIT_FAILURE);
        }
    }
    board->state_stack[board->ply] = next_state;
    // update hash for castling
    prev_state = (prev_state >> 27) & 0xf;
    next_state = (next_state >> 27) & 0xf;
    board->hash ^= ZOBRIST_CASTLING[prev_state];
    board->hash ^= ZOBRIST_CASTLING[next_state];
}

void unmake_move(Board *board, Move move) {
    U64 from = 1ULL << get_from(move);
    U64 to = 1ULL << get_to(move);
    U64 aux1, aux2;
    board->side_to_move ^= 1;
    board->hash ^= ZOBRIST_BLACK; // update hash
    bool curr_color = board->side_to_move;
    int i, captured_piece_idx;
    StateFlags prev_state = board->state_stack[board->ply-1];
    StateFlags next_state = board->state_stack[board->ply  ];

    if (next_state & 0x04000000) {
        // update hash to remove previous ep target
        board->hash ^= ZOBRIST_EP[((next_state >> 20) & 0x3f) % 8];
    }

    if (prev_state & 0x04000000) {
        // update hash to remove previous ep target
        board->hash ^= ZOBRIST_EP[((prev_state >> 20) & 0x3f) % 8];
    }

    board->ply--;

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

        board->hash ^= ZOBRIST_PIECE_SQ[ROOK_IDX][curr_color][LOG2(from)]; // update hash
        board->hash ^= ZOBRIST_PIECE_SQ[KING_IDX][curr_color][LOG2(aux1)];
        board->hash ^= ZOBRIST_PIECE_SQ[ROOK_IDX][curr_color][LOG2(to)];
        board->hash ^= ZOBRIST_PIECE_SQ[KING_IDX][curr_color][LOG2(aux2)];

        goto end;
    }


    for (i = 0; i < NUM_PIECES; i++) {
        if (board->pieces[i] & to) {
            break;
        }
    }

    board->pieces[i] &= ~to;
    board->colors[curr_color] &= ~to;
    board->hash ^= ZOBRIST_PIECE_SQ[i][curr_color][LOG2(to)]; // update hash
    
    if (move >> 12 == 5) { // check if ep capture
        aux1 = curr_color ? nort_one(to) : sout_one(to); // ep-captured pawn
        
        board->pieces[PAWN_IDX] |= aux1;
        board->colors[curr_color ^ 1] |= aux1;

        board->hash ^= ZOBRIST_PIECE_SQ[PAWN_IDX][curr_color ^ 1][LOG2(aux1)]; // update hash
    } else if (move & 0x4000) { // check if capture
        captured_piece_idx = (next_state >> 17) & 0x07; // restore captured piece from state_stack

        board->pieces[captured_piece_idx] |= to;
        board->colors[curr_color ^ 1] |= to;

        board->hash ^= ZOBRIST_PIECE_SQ[captured_piece_idx][curr_color ^ 1][LOG2(to)]; // update hash
    }


    if (move & 0x8000) // check if promotion
        i = PAWN_IDX;

    board->pieces[i] |= from;
    board->colors[curr_color] |= from;
    board->hash ^= ZOBRIST_PIECE_SQ[i][curr_color][LOG2(from)]; // update hash
end:
    // update hash for castling
    next_state = (next_state >> 27) & 0xf;
    prev_state = (prev_state >> 27) & 0xf;
    board->hash ^= ZOBRIST_CASTLING[next_state];
    board->hash ^= ZOBRIST_CASTLING[prev_state];
}

Move* legal_moves(Board *board, Move *given) {
    Move *list = given;
    U64 aux1, aux2, aux3, aux4;
    Sq from;
    Move move;
    int j;
    bool curr_side = board->side_to_move;
    U64 friendly = board->colors[curr_side];
    U64 enemy = board->colors[curr_side ^ 1];
    U64 king = board->pieces[KING_IDX] & friendly;
    U64 ds = danger_squares(board);
    U64 checkers = get_checkers(board, curr_side);
    U64 pins = get_pins(board, curr_side);
    U64 capture_mask = -1;
    U64 push_mask = -1;

    // king moves
    from = LOG2(king);
    aux2 = k_moves(king) & ~ds & ~friendly;
    while (aux2) {
        aux1 = pop_lsb(&aux2);
        move = new_move(from, LOG2(aux1), aux1 & enemy ? 4 : 0);
        *(list++) = move;
    }

    if (POP_COUNT(checkers) > 1) { // double check or more
        goto end; // no other pieces can move
    } else if (checkers) { // single check, create capture & push mask
        capture_mask = checkers;
        push_mask = ray_between(board->pieces[KING_IDX] & friendly, checkers);
    } else if (king & (curr_side ? 0x1000000000000000ULL : 0x0000000000000010)) { // if king is at home square
        aux1 = curr_side ? 0x6e00000000000000ULL : 0x000000000000006eULL; // castle masks
        aux2 = 0x6000000000000060ULL & aux1;
        aux3 = curr_side ? 0x8000000000000000ULL : 0x0000000000000080ULL; // rook short castle home
        if (can_castle(board, curr_side, 0) && !(aux2 & (ds | friendly | enemy)) && (aux3 & board->pieces[ROOK_IDX])) // short_castle
            *(list++) = curr_side ? MOVE_B_SHORT_CASTLE : MOVE_W_SHORT_CASTLE;

        aux2 = 0x0c0000000000000cULL & aux1;
        aux3 = curr_side ? 0x0100000000000000ULL : 0x0000000000000001ULL; // rook long castle home
        aux4 = curr_side ? 0x0200000000000000ULL : 0x0000000000000002ULL; // extra check outside of castle mask
        if (can_castle(board, curr_side, 1) && !(aux2 & (ds | friendly | enemy)) && (aux3 & board->pieces[ROOK_IDX]) && !((friendly | enemy) & aux4)) // long_castle
            *(list++) = curr_side ? MOVE_B_LONG_CASTLE : MOVE_W_LONG_CASTLE;
    }

    // pawn moves
    aux1 = board->pieces[PAWN_IDX] & friendly;
    while (aux1) {
        aux2 = pop_lsb(&aux1);
        aux3 = (curr_side ? sout_one(aux2) : nort_one(aux2)) & ~(enemy | friendly);

        if (aux2 & pins) // if pawn is pinned
            aux3 &= v_moves(king, (friendly | enemy) & ~aux2);

        if (aux3 & (RANK_1 | RANK_8) & push_mask) { // if pawn promotion
            for (j = 0; j < 4; j++) {
                move = new_move(LOG2(aux2), LOG2(aux3), PROMOTE_N + j);
                *(list++) = move;
            }
        } else if (aux3) { // single push
            if (aux3 & push_mask)
                *(list++) = new_move(LOG2(aux2), LOG2(aux3), 0);

            aux4 = (curr_side ? sout_one(aux3) : nort_one(aux3)) & ~(enemy | friendly) & push_mask;
            if (aux4 & (curr_side ? RANK_5 : RANK_4)) { // double push
                move = new_move(LOG2(aux2), LOG2(aux4), DOUBLE_PUSH);
                *(list++) = move;
            }
        }
        U64 ep_targ = ep_target(board);
        aux4 = curr_side ? sout_one(aux2) : nort_one(aux2);
        aux3 = (east_one(aux4) | west_one(aux4)) & (enemy | ep_targ);
        while (aux3) {
            aux4 = pop_lsb(&aux3);
            U64 captured_ep_mask = curr_side ? nort_one(ep_targ) : sout_one(ep_targ);
            if ((aux4 & ep_targ) && ((captured_ep_mask & capture_mask) || (aux4 & push_mask))) {
                U64 ep_discover_mask = ((friendly | enemy) & ~(captured_ep_mask | aux2)) | aux4;
                if (!(r_moves(king, ep_discover_mask) & enemy & (board->pieces[QUEEN_IDX] | board->pieces[ROOK_IDX]))) { // if not ep discovered check
                    if (!(aux2 & pins) || (b_moves(king, (friendly | enemy) & ~aux2) & aux4)) {
                        move = new_move(LOG2(aux2), LOG2(aux4), EP_CAPTURE);
                        *(list++) = move;
                    }
                }
            } else if ((aux4 & (push_mask | capture_mask)) && (!(aux2 & pins) || ((b_moves(king, (friendly | enemy) & ~aux2) & aux4)
                            && !(r_moves(king, (friendly | enemy) & ~aux2) & (board->pieces[ROOK_IDX] | board->pieces[QUEEN_IDX]) & enemy)))) { // FOR THE LOVE OF GOD FIX ME
                if (aux4 & (RANK_1 | RANK_8)) {
                    for (j = 0; j < 4; j++) {
                        move = new_move(LOG2(aux2), LOG2(aux4), PROMOTE_CAPTURE_N + j);
                        *(list++) = move;
                    }
                } else {
                    move = new_move(LOG2(aux2), LOG2(aux4), 4);
                    *(list++) = move;
                }
            }
        }
    }

    // knight moves
    aux1 = board->pieces[KNIGHT_IDX] & friendly;
    while (aux1) {
        aux2 = pop_lsb(&aux1);
        aux3 = n_moves(aux2) & ~friendly;
        while (aux3 && !(aux2 & pins)) {
            aux4 = pop_lsb(&aux3);
            if ((aux4 & push_mask) || (aux4 & capture_mask))
                *(list++) = new_move(LOG2(aux2), LOG2(aux4), aux4 & enemy ? 4 : 0);
        }
    }
    
    aux1 = board->pieces[BISHOP_IDX] & friendly;
    while (aux1) {
        aux2 = pop_lsb(&aux1);
        aux3 = b_moves(aux2, friendly | enemy) & ~friendly;

        if (aux2 & pins) {
            U64 pin_rays = b_moves(king, (friendly | enemy) & ~aux2);
            if (!(pin_rays & enemy & (board->pieces[BISHOP_IDX] | board->pieces[QUEEN_IDX])))
                pin_rays &= 0ULL;

            aux3 &= pin_rays;
        }

        while (aux3) {
            aux4 = pop_lsb(&aux3);
            if ((aux4 & push_mask) || (aux4 & capture_mask))
                *(list++) = new_move(LOG2(aux2), LOG2(aux4), aux4 & enemy ? 4 : 0);
        }
    }
    
    aux1 = board->pieces[ROOK_IDX] & friendly;
    while (aux1) {
        aux2 = pop_lsb(&aux1);
        aux3 = r_moves(aux2, friendly | enemy) & ~friendly;

        if (aux2 & pins) {
            U64 pin_rays = r_moves(king, (friendly | enemy) & ~aux2);
            if (!(pin_rays & enemy & (board->pieces[ROOK_IDX] | board->pieces[QUEEN_IDX])))
                pin_rays &= 0ULL;

            aux3 &= pin_rays;
        }

        while (aux3) {
            aux4 = pop_lsb(&aux3);
            if ((aux4 & push_mask) || (aux4 & capture_mask))
                *(list++) = new_move(LOG2(aux2), LOG2(aux4), aux4 & enemy ? 4 : 0);
        }
    }
    
    aux1 = board->pieces[QUEEN_IDX] & friendly;
    while (aux1) {
        aux2 = pop_lsb(&aux1);
        aux3 = q_moves(aux2, friendly | enemy) & ~friendly;

        if (aux2 & pins) {
            aux4 = r_moves(king, (friendly | enemy) & ~aux2);
            if (aux4 & (board->pieces[ROOK_IDX] | board->pieces[QUEEN_IDX]) & ~checkers & enemy) {
                aux3 = aux4 & r_moves(aux2, friendly | enemy) & ~friendly;
            } else {
                aux4 = b_moves(king, (friendly | enemy) & ~aux2);
                aux3 &= aux4 & b_moves(aux2, friendly | enemy) & ~friendly;
            }
        }

        while (aux3) {
            aux4 = pop_lsb(&aux3);
            if ((aux4 & push_mask) || (aux4 & capture_mask))
                *(list++) = new_move(LOG2(aux2), LOG2(aux4), aux4 & enemy ? 4 : 0);
        }
    }
    
end:
    *list = (list - given);
    return list;
}

bool is_in_check(Board *board) {
    return get_checkers(board, board->side_to_move) != 0;
}

Move random_move(Board *board) {
    Move *curr = (Move[256]){0};
    Move *end = legal_moves(board, curr);

    if (curr == end)
        return 0;

    return *(curr + (rand() % *end));
    
}

Move move_from_str(Board *board, char* str) {
    U64 from_bb, to_bb;
    MoveFlags flags = 0;
    char promote = str[4];
    Sq to, from = sq_from_str(str);
    str += 2;
    to = sq_from_str(str);
    from_bb = 1ULL << from;
    to_bb = 1ULL << to;
    
    if ((1ULL << from) & ((1ULL << e1) | (1ULL << e8)) & board->pieces[KING_IDX]) { // castling
        if (from == e1) {
            if (to == g1) {
                return MOVE_W_SHORT_CASTLE;
            } else if (to == c1) {
                return MOVE_W_LONG_CASTLE;
            }
        } else {
            if (to == g8) {
                return MOVE_B_SHORT_CASTLE;
            } else if (to == c8) {
                return MOVE_B_LONG_CASTLE;
            }
        }
    }

    if (to_bb & (board->colors[WHITE] | board->colors[BLACK])) // capture
        flags = 4;
    
    if ((from_bb & board->pieces[PAWN_IDX]) && (to_bb & ep_target(board))) // ep capture
        flags = 5;

    if ((from_bb & board->pieces[PAWN_IDX] & (RANK_2 | RANK_7)) && (to_bb & (RANK_4 | RANK_5))) // double pawn push
        flags = 1;

    if (promote)
        flags |= 8;

    switch (promote) {
        case 'b':
            flags |= 1;
            break;
        case 'r':
            flags |= 2;
            break;
        case 'q':
            flags |= 3;
            break;
    }

    return new_move(from, to, flags);
}

U64 perft(Board *board, int depth) {
    if (depth == 0)
        return 1;

#if DEBUG
    Board *copy;
#endif
    Move *curr = (Move[256]){0};
    Move *end = legal_moves(board, curr);
    U64 nodes = 0;

    if (depth == 1)
        return *end;

    while (curr < end) {
#if DEBUG
        copy = copy_board(board);
#endif
        make_move(board, *curr);
        nodes += perft(board, depth-1);
        unmake_move(board, *curr);
#if DEBUG
        if (!boards_equal(board, copy)) {
            printf("\nMAKE MOVE != UNMAKE MOVE FOR:");
            print_move(*curr);
            printf(" %lx\n", *curr >> 12);
            print_board(copy);
            printf("\n");
            print_board(board);
            exit(EXIT_FAILURE);
        }
        free_board(copy);
#endif
        curr++;
    }
    
    return nodes;
}

void print_perft(Board *board, int depth) {
#if DEBUG
    Board *copy;
#endif
    //clock_t start = clock(), end;
    //double duration;
    Move *curr = (Move[256]){0};
    Move *end = legal_moves(board, curr);
    U64 total_nodes = 0;
    U64 curr_node = 0;
    
    while (curr < end) {
#if DEBUG
        copy = copy_board(board);
#endif
        make_move(board, *curr);
        print_move(*curr);
        printf(": ");
        curr_node = perft(board, depth-1);
        printf("%lu\n", curr_node);
        total_nodes += curr_node;
        unmake_move(board, *curr);
#if DEBUG
        if (!boards_equal(board, copy)) {
            printf("\nMAKE MOVE != UNMAKE MOVE FOR:");
            print_move(*curr);
            printf(" %lx\n", *curr >> 12);
            print_board(copy);
            printf("\n");
            print_board(board);
            exit(EXIT_FAILURE);
        }
        free_board(copy);
#endif
        curr++;
    }
    //end = clock();
    //duration = (double)(end - start) / CLOCKS_PER_SEC;
    //printf("\nTotal nodes: %lu (%d nps)\n", total_nodes, (int)(total_nodes / duration));
    printf("\nTotal nodes: %lu\n", total_nodes);
}


Board* from_fen(char* fen) {
    Board *board = (Board*)malloc(sizeof(Board));
    memset(board, 0, sizeof(Board));
    board->stack_capacity = INITIAL_MOVE_CAPACITY;
    board->state_stack = (StateFlags *)malloc(board->stack_capacity * sizeof(StateFlags));
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
    //debug_assert(j == 63, "FEN translation for piece placement must equal 64");

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

    if (fen[i] != '-') // set ep square
        board->state_stack[board->ply] |= (sq_from_str(fen + i++) << 20) | (1 << 26);

    i += 2;

    board->state_stack[board->ply] |= (atoi(fen + i) & 0x1ffff);
    for (; fen[i] != ' '; i++);
    i++;

    board->ply_offset = atoi(fen + i) * 2 - 2;
    if (board->side_to_move == BLACK)
        board->ply_offset++;
    for (; fen[i] != ' '; i++);

    board->hash = board_hash(board);

    return board;
}

char* to_fen(Board *board) {
    char *ptr, *fen = malloc(sizeof(char) * 128);
    char to_add;
    int i, piece, color, gap = 0;
    bool found;
    U64 d_bb;
    Sq sq;
    StateFlags state = board->state_stack[board->ply];

    ptr = fen;

    for (i = 0; i < NUM_SQUARES; i++) {
        d_bb = 1ULL << ((i%8) + (7-i/8)*8);
        found = false;
        for (color = 0; color < 2 && !found; color++) {
            for (piece = 0; piece < NUM_PIECES && !found; piece++) {
                if (board->colors[color] & board->pieces[piece] & d_bb) {
                    found = true;
                }
            }
        }
        color--;
        piece--;
        if (found) {
            if (gap) {
                *ptr = gap+48;
                gap = 0;
                ptr++;
            }

            switch (piece) {
                case PAWN_IDX:
                    to_add = 'p';
                    break;
                case KNIGHT_IDX:
                    to_add = 'n';
                    break;
                case BISHOP_IDX:
                    to_add = 'b';
                    break;
                case ROOK_IDX:
                    to_add = 'r';
                    break;
                case QUEEN_IDX:
                    to_add = 'q';
                    break;
                case KING_IDX:
                    to_add = 'k';
                    break;
                default:
                    to_add = '?';
            }
            if (color == WHITE)
                to_add = toupper(to_add);
            
            *ptr = to_add;
            ptr++;
        } else {
            gap++;
        }

        if (i % 8 == 7) {
            if (gap) {
                *ptr = gap+48;
                gap = 0;
                ptr++;
            }

            if (i / 8 != 7)
                *(ptr++) = '/';
        }
    }

    *(ptr++) = ' ';
    *(ptr++) = board->side_to_move == WHITE ? 'w' : 'b';
    *(ptr++) = ' ';

    found = false;
    if (can_castle(board, WHITE, KINGSIDE)) {
        found = true;
        *(ptr++) = 'K';
    }
    if (can_castle(board, WHITE, QUEENSIDE)) {
        found = true;
        *(ptr++) = 'Q';
    }
    if (can_castle(board, BLACK, KINGSIDE)) {
        found = true;
        *(ptr++) = 'k';
    }
    if (can_castle(board, BLACK, QUEENSIDE)) {
        found = true;
        *(ptr++) = 'q';
    }
    if (!found)
        *(ptr++) = '-';
    
    *(ptr++) = ' ';
    if (TEST_BIT(state, 26)) {
        sq = (state >> 20) & 0x3f;
        *(ptr++) = 0x61+(sq%8);
        *(ptr++) = sq/8+49;
    } else {
        *(ptr++) = '-';
    }

    sprintf(ptr, " %d %d", half_moves(board), ((board->ply + board->ply_offset) / 2) + 1);


    return fen;
}

void free_board(Board *board) {
    free(board->state_stack);
    free(board);
}

void print_board_bb(Board *board) {
    printf("\nWHITE\n");
    print_bb(board->colors[WHITE]);
    printf("\nBLACK\n");
    print_bb(board->colors[BLACK]);

    printf("\n\nPAWNS\n");
    print_bb(board->pieces[PAWN_IDX]);
    printf("\nKNIGHTS\n");
    print_bb(board->pieces[KNIGHT_IDX]);
    printf("\nBISHOPS\n");
    print_bb(board->pieces[BISHOP_IDX]);
    printf("\nROOKS\n");
    print_bb(board->pieces[ROOK_IDX]);
    printf("\nQUEENS\n");
    print_bb(board->pieces[QUEEN_IDX]);
    printf("\nKINGS\n");
    print_bb(board->pieces[KING_IDX]);
}

void print_board(Board *board) {
    StateFlags state = board->state_stack[board->ply];
    U64 bb;
    int i, j, k, c, color, piece;
    char castle_rights[5];
    
    printf("           %c to move\n", board->side_to_move ? 'b' : 'w');
    printf("       ");
    for (i = 0; i < 8; i++) {
        printf("%c ", 0x61 + i);
    }
    printf("\n");
    for (i = 7; i >= 0; i--) {
        printf("     %d ", i+1);
        for (j = 0; j < 8; j++) {
            c = i % 2 != j % 2 ? '.' : ',';
            for (k = 0; k < NUM_PIECES * NUM_COLORS; k++) {
                piece = k % NUM_PIECES;
                color = k / NUM_PIECES;
                bb = board->pieces[piece] & board->colors[color];
                
                if ((bb >> (j+i*8)) & 1ULL) {
                    switch (piece) {
                        case PAWN_IDX:
                            c = 'p';
                            break;
                        case KNIGHT_IDX:
                            c = 'n';
                            break;
                        case BISHOP_IDX:
                            c = 'b';
                            break;
                        case ROOK_IDX:
                            c = 'r';
                            break;
                        case QUEEN_IDX:
                            c = 'q';
                            break;
                        case KING_IDX:
                            c = 'k';
                            break;
                        default:
                            c = '?';
                    }
                    c -= color ? 0 : 0x20;
                    break;
                }
            }
            printf("%c ", c);
        }
        printf("%d\n", i+1);
    }
    printf("       ");
    for (i = 0; i < 8; i++) {
        printf("%c ", 0x61 + i);
    }
    printf("\n     ply %15d", board->ply + board->ply_offset);
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
    printf("\n     castle %12s", castle_rights);
    printf("\n     half_move %9d", half_moves(board));
    printf("\n     ep_target %7s", "");
    if (TEST_BIT(state, 26)) {
        print_sq((state >> 20) & 0x3f);
    } else {
        printf(" -");
    }
    printf("\n");
#if DEBUG
    /*for (i = 0; i <= board->ply; i++) {
        printf("%x\n", board->state_stack[i]);
    }
    printf("\n");*/
#endif
}

void wprint_board(Board *board) {
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
    /*Move *moves = legal_moves(board);
    wprint_move_buffer(moves);
    wprintf(L"\n");*/
}

/*void print_move_buffer(Move *buffer) {
    for (; *buffer; buffer++) {
        printf("\n");
        print_move(*buffer);
    }
    printf("\n");
}


void wprint_move_buffer(Move *buffer) {
    for (; *buffer; buffer++) {
        wprintf(L"\n");
        wprint_move(*buffer);
    }
    wprintf(L"\n");
}*/
