#include "movegen.h"
#include "types.h"

static U64 N_MOVE_TABLE[NUM_SQUARES];

static U64 n_moves_slow(U64 orig) {
    // source
    // https://www.chessprogramming.org/Knight_Pattern
    
    U64 west, east, attacks;
    east     = east_one(orig);
    west     = west_one(orig);
    attacks  = (east | west) << 16;
    attacks |= (east | west) >> 16;
    east     = east_one(east);
    west     = west_one(west);
    attacks |= (east | west) << 8;
    attacks |= (east | west) >> 8;

    return attacks;
}

void init_n_moves() {
    int i = 0;

    for (i = 0; i < NUM_SQUARES; i++) {
        N_MOVE_TABLE[i] = n_moves_slow(1ULL << (63 - i));
    }
}

U64 n_moves(U64 orig) {
    return N_MOVE_TABLE[__builtin_clzll(orig)];
}

