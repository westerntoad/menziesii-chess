#include "table.h"
#include "utils.h"

static U64 ZOBRIST_PIECE_SQ[NUM_PIECES][NUM_SQUARES];
static U64 ZOBRIST_SIDE[NUM_COLORS];
static U64 ZOBRIST_CASTLING[16];
static U64 ZOBRIST_EP[8];

void init_table() {
    int i, j;
    psrng_u64_seed(0ULL);

    for (i = 0; i < NUM_PIECES; i++) {
        for (j = 0; j < NUM_SQUARES; j++) {
            ZOBRIST_PIECE_SQ[i][j] = psrng_u64();
        }
    }

    ZOBRIST_SIDE[WHITE] = psrng_u64();
    ZOBRIST_SIDE[BLACK] = psrng_u64();

    for (i = 0; i < 16; i++) {
        ZOBRIST_CASTLING[i] = psrng_u64();
    }

    for (i = 0; i < 8; i++) {
        ZOBRIST_EP[i] = psrng_u64();
    }
}

U64 board_hash(Board* board) {
    U64 hash = 0ULL;
    U64 bb;
    Sq sq;
    StateFlags state = board->state_stack[board->ply];

    for (int i = 0; i < NUM_PIECES; i++) {
        bb = board->pieces[i];

        while (bb) {
            sq = LOG2(pop_lsb(&bb));
            hash ^= ZOBRIST_PIECE_SQ[i][sq];
        }
    }

    hash ^= ZOBRIST_SIDE[board->side_to_move];
    hash ^= ZOBRIST_CASTLING[(state >> 27) & 0xf];
    if (state & 0x04000000)
        hash ^= ZOBRIST_EP[((state >> 20) & 0x3f) % 8];

    return hash;
}

void print_table() {
    int i, j;

    for (i = 0; i < NUM_PIECES; i++) {
        for (j = 0; j < NUM_SQUARES; j++) {
            printf("%lx\n", ZOBRIST_PIECE_SQ[i][j]);
        }
    }

    printf("%lx\n", ZOBRIST_SIDE[WHITE]);
    printf("%lx\n", ZOBRIST_SIDE[BLACK]);

    for (i = 0; i < 16; i++) {
        printf("%lx\n", ZOBRIST_CASTLING[i]);
    }

    for (i = 0; i < 8; i++) {
        printf("%lx\n", ZOBRIST_EP[i]);
    }
}
