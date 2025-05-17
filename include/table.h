#ifndef TABLE_H // include guard
#define TABLE_H

#include "board.h"
#include "types.h"

#define EMPTY_NODE '\0'
#define EXACT_NODE 'e'
#define UPPER_BOUND_NODE 'a'
#define LOWER_BOUND_NODE 'b'

typedef struct {
    U64  key;
    char type;
    U8   depth;
    int  score;
    Move best;
} TTEntry;

// Zobrist hashes
extern U64 ZOBRIST_PIECE_SQ[NUM_PIECES][NUM_SQUARES];
extern U64 ZOBRIST_BLACK;
// 1111 -> KQkq
extern U64 ZOBRIST_CASTLING[16];
extern U64 ZOBRIST_EP[8];

void init_zobrist();
void tt_set_size(int mb_size);
TTEntry* tt_probe(U64 key);
void tt_save(U64 key, U8 depth, int score, Move best, char type);
U64 board_hash(Board* board);
void print_table();

#endif  // TABLE_H
