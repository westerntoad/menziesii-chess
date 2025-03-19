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

#define PAWN_IDX     0
#define KNIGHT_IDX   1
#define BISHOP_IDX   2
#define ROOK_IDX     3
#define QUEEN_IDX    4
#define KING_IDX     5

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
/*
 * A flag determining metadata for a single move
 * https://www.chessprogramming.org/Encoding_Moves
 *
 * 0000 quiet move
 * 0001 double pawn push
 * 0010 short castle
 * 0011 long castle
 * 0100 capture
 * 0101 ep-capture
 * 1000 n promotion
 * 1001 b promotion
 * 1010 r promotion
 * 1011 q promotion
 * 1100 n capture-promotion
 * 1101 b capture-promotion
 * 1110 r capture-promotion
 * 1111 q capture-promotion
 */
typedef uint_fast8_t MoveFlags;
/*
 * A flag storing all irreversible effects of the current chess position. This is
 * held in an array of flags to allow unmake_move to function.
 *
 * v                                       has-data bit
 * 0000 0000 0000 0000 0000 0000 0000 0000 
 *  v---v                                  castle bits (KQkq)
 * 0000 0000 0000 0000 0000 0000 0000 0000 
 *       v                                 ep target exist bit
 * 0000 0000 0000 0000 0000 0000 0000 0000 
 *        v-----v                          ep target square-index
 * 0000 0000 0000 0000 0000 0000 0000 0000 
 *                v-v                      captured piece index ONE-INDEXED (pawn = 1, rook = 4, etc)
 * 0000 0000 0000 0000 0000 0000 0000 0000
 *                   v------v              captured piece previous square
 * 0000 0000 0000 0000 0000 0000 0000 0000 
 *                           v-----------v half-move clock
 * 0000 0000 0000 0000 0000 0000 0000 0000 
 */
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
