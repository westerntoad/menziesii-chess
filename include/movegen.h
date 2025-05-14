#ifndef MOVEGEN_H // include guard
#define MOVEGEN_H

#include "types.h"

void init_move_lookup_tables();
U64 n_moves(U64 orig);
U64 k_moves(U64 orig);
U64 r_moves(U64 orig, U64 blocks);
U64 b_moves(U64 orig, U64 blocks);
//U64 h_moves(U64 orig, U64 blocks);
U64 v_moves(U64 orig, U64 blocks);
U64 q_moves(U64 orig, U64 blocks);

#endif  // MOVEGEN_H
