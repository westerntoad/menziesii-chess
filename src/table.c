#include <string.h>
#include "table.h"
#include "types.h"
#include "utils.h"

static TTEntry* T_TABLE = NULL;
static int TT_ENTRIES = 0;

// Zobrist hashes
U64 ZOBRIST_PIECE_SQ[NUM_PIECES][NUM_SQUARES];
U64 ZOBRIST_BLACK;
U64 ZOBRIST_CASTLING[16];
U64 ZOBRIST_EP[8];

void init_zobrist() {
    int i, j;
    psrng_u64_seed(0ULL);

    for (i = 0; i < NUM_PIECES; i++) {
        for (j = 0; j < NUM_SQUARES; j++) {
            ZOBRIST_PIECE_SQ[i][j] = psrng_u64();
            //ZOBRIST_PIECE_SQ[i][j] = 0;
        }
    }

    ZOBRIST_BLACK = psrng_u64();
    //ZOBRIST_BLACK = 0;

    for (i = 0; i < 16; i++) {
        ZOBRIST_CASTLING[i] = psrng_u64();
        //ZOBRIST_CASTLING[i] = 0;
    }

    for (i = 0; i < 8; i++) {
        ZOBRIST_EP[i] = psrng_u64();
        //ZOBRIST_EP[i] = 0;
    }
}

void tt_set_size(int mb_size) {
    if (T_TABLE)
        free(T_TABLE);
    
    int byte_size = mb_size * 1024 * 1024;
    TT_ENTRIES = byte_size / sizeof(TTEntry);
    T_TABLE = malloc(sizeof(TTEntry) * TT_ENTRIES);
    if (T_TABLE == NULL) {
        fprintf(stderr, "Error allocating space for transposition table of size %dmb.\n", mb_size);
        TT_ENTRIES = 0;
    } else {
        memset(T_TABLE, 0, TT_ENTRIES * sizeof(TTEntry));
    }
}

TTEntry* tt_probe(U64 key) {
    if (!TT_ENTRIES)
        return NULL;

    TTEntry* entry = &T_TABLE[key % TT_ENTRIES];

    if (entry->key == key)
        return entry;

    return NULL;
}

void tt_save(U64 key, U8 depth, int score, Move best, char type) {
    if (!TT_ENTRIES)
        return;
    
    TTEntry* entry = &T_TABLE[key % TT_ENTRIES];

    if ((entry->key == key) && (entry->depth > depth)) return;

    entry->key = key;
    entry->depth = depth;
    entry->score= score;
    entry->best = best;
    entry->type = type;
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

    if (board->side_to_move == BLACK)
        hash ^= ZOBRIST_BLACK;

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

    printf("%lx\n", ZOBRIST_BLACK);

    for (i = 0; i < 16; i++) {
        printf("%lx\n", ZOBRIST_CASTLING[i]);
    }

    for (i = 0; i < 8; i++) {
        printf("%lx\n", ZOBRIST_EP[i]);
    }
}
