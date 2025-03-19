#ifndef TYPES_H // include guard
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>

#define NUM_SQUARES 64
#define NUM_COLORS  2
#define NUM_PIECES  6

#define WHITE false
#define BLACK true
#define KINGSIDE false
#define QUEENSIDE true

#define PAWN_IDX   0
#define KNIGHT_IDX 1
#define BISHOP_IDX 2
#define ROOK_IDX   3
#define QUEEN_IDX  4
#define KING_IDX   5

#define A_FILE 0x0101010101010101ULL 
#define H_FILE 0x8080808080808080ULL 

typedef uint64_t U64;
typedef uint_fast8_t Sq;


/*
 * A single move with to-from squares & addition move flags
 * v--v                 flags
 * 0000 0000 0000 0000
 *      v-----v         from
 * 0000 0000 0000 0000
 *             v-----v  to
 * 0000 0000 0000 0000
 */
typedef uint_fast16_t Move;
typedef uint_fast8_t MoveFlags;
typedef uint32_t StateFlags;

Move new_move(Sq from, Sq to, MoveFlags flags);
Sq get_from(Move move);
Sq get_to(Move move);

void print_bb(U64 bb);
U64 nort_one(U64 bb);
U64 east_one(U64 bb);
U64 sout_one(U64 bb);
U64 west_one(U64 bb);
U64 delta_one(U64 bb, int dx, int dy);

#endif  // TYPES_H
