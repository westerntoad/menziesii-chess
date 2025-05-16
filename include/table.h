#ifndef TABLE_H // include guard
#define TABLE_H

#include "board.h"
#include "types.h"

void init_table();
U64 board_hash(Board* board);
void print_table();

#endif  // TABLE_H
