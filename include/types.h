#ifndef TYPES_H // include guard
#define TYPES_H

#include <stdint.h>
#include <stdlib.h>
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

#define UNIVERSE 0xffffffffffffffffULL 
#define A_FILE   0x0101010101010101ULL 
#define H_FILE   0x8080808080808080ULL 
#define RANK_1   0x00000000000000ffULL
#define RANK_2   0x000000000000ff00ULL
#define RANK_4   0x00000000ff000000ULL
#define RANK_5   0x000000ff00000000ULL
#define RANK_7   0x00ff000000000000ULL
#define RANK_8   0xff00000000000000ULL

#define MOVE_W_SHORT_CASTLE 0x2106
#define MOVE_W_LONG_CASTLE  0x3102
#define MOVE_B_SHORT_CASTLE 0x2f3e
#define MOVE_B_LONG_CASTLE  0x3f3a

#define DOUBLE_PUSH       0x1
#define EP_CAPTURE        0x5
#define PROMOTE_N         0x8
#define PROMOTE_N         0x8
#define PROMOTE_B         0x9
#define PROMOTE_R         0xa
#define PROMOTE_Q         0xb
#define PROMOTE_CAPTURE_N 0xc
#define PROMOTE_CAPTURE_B 0xd
#define PROMOTE_CAPTURE_R 0xe
#define PROMOTE_CAPTURE_Q 0xf

// https://www.chessprogramming.org/Square_Mapping_Considerations
enum enumSquare {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8
};

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
 * 0101 ep capture
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
 *                v-v                      captured piece index (pawn = 0, rook = 3, etc)
 * 0000 0000 0000 0000 0000 0000 0000 0000
 *                   v-------------------v half-move clock
 * 0000 0000 0000 0000 0000 0000 0000 0000 
 */
typedef uint32_t StateFlags;
typedef struct {
    Move *data;
    size_t size;
    size_t capacity;
} MoveList;

MoveList *new_movelist();
void free_movelist(MoveList *list);
int push_move(MoveList *list, Move move);
Move pop_move(MoveList *list);
void print_movelist(MoveList *list);

Sq flip_v(Sq sq);
Sq sq_from_str(char *s);
void print_sq(Sq sq);
void wprint_sq(Sq sq);

Move new_move(Sq from, Sq to, MoveFlags flags);
Sq get_from(Move move);
Sq get_to(Move move);
bool is_promotion(Move move);
bool is_capture(Move move);
void print_move(Move move);
void wprint_move(Move move);

U64 pop_lsb(U64 *bb);
U64 nort_one(U64 bb);
U64 east_one(U64 bb);
U64 sout_one(U64 bb);
U64 west_one(U64 bb);
U64 delta_one(U64 bb, int dx, int dy);
void print_bb(U64 bb);
void wprint_bb(U64 bb);

#endif  // TYPES_H
