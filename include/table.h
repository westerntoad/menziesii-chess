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
    int  depth;
    int  score;
    Move best;
} TTEntry;

void init_zobrist();
void tt_set_size(int mb_size);
TTEntry* tt_probe(U64 key);
void tt_save(U64 key, int depth, int score, Move best, char type);
U64 board_hash(Board* board);
void print_table();

#endif  // TABLE_H
