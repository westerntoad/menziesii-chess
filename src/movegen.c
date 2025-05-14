#include "movegen.h"
#include "utils.h"

static U64 N_MOVE_TABLE[NUM_SQUARES];
static U64 K_MOVE_TABLE[NUM_SQUARES];

static U64 n_moves_slow(U64 orig) {
    // source
    // https://www.chessprogramming.org/Knight_Pattern
    
    U64 east, west, attacks;
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

static U64 k_moves_slow(U64 orig) {
    U64 attacks;
    attacks  = east_one(orig) | west_one(orig);
    attacks |= nort_one(attacks) | sout_one(attacks);
    attacks |= nort_one(orig);
    attacks |= sout_one(orig);

    return attacks;
}

static U64 sliding_attacks(U64 orig, U64 blocks, const int dxes[], const int dyes[], int length) {
    U64 dbb, attacks = 0ULL;
    int i;

    for (i = 0; i < length; i++) {
        dbb = orig;
        while (dbb) {
            dbb = delta_one(dbb, dxes[i], dyes[i]);
            attacks |= dbb;

            if (dbb & blocks) break;
        }
    }

    return attacks;
}

void init_move_lookup_tables() {
    int i = 0;

    for (i = 0; i < NUM_SQUARES; i++) {
        U64 bb = 1ULL << i;
        N_MOVE_TABLE[i] = n_moves_slow(bb);
        K_MOVE_TABLE[i] = k_moves_slow(bb);
    }
}

U64 n_moves(U64 orig) {
    return N_MOVE_TABLE[LOG2(orig)];
}

U64 k_moves(U64 orig) {
    return K_MOVE_TABLE[LOG2(orig)];
}

U64 r_moves(U64 orig, U64 blocks) {
    const int dxes[4] = { 0,  1,  0, -1 };
    const int dyes[4] = { 1,  0, -1,  0 };

    return sliding_attacks(orig, blocks, dxes, dyes, 4);
}

U64 b_moves(U64 orig, U64 blocks) {
    const int dxes[4] = { 1, -1,  1, -1 };
    const int dyes[4] = { 1,  1, -1, -1 };

    return sliding_attacks(orig, blocks, dxes, dyes, 4);
}

U64 h_moves(U64 orig, U64 blocks) {
    const int dxes[4] = { 0,  1 };
    const int dyes[4] = { 0, -1 };

    return sliding_attacks(orig, blocks, dxes, dyes, 2);
}

U64 v_moves(U64 orig, U64 blocks) {
    const int dxes[2] = { 0,  0 };
    const int dyes[2] = { 1, -1 };

    return sliding_attacks(orig, blocks, dxes, dyes, 2);
}

U64 q_moves(U64 orig, U64 blocks) {
    return r_moves(orig, blocks) | b_moves(orig, blocks);
}
