#include <string.h>
#include "eval.h"
#include "table.h"
#include "types.h"
#include "utils.h"

static TTEntry* T_TABLE = NULL;
static U64 TT_ENTRIES = 0;

// Zobrist hashes
U64 ZOBRIST_PIECE_SQ[NUM_PIECES][NUM_COLORS][NUM_SQUARES];
U64 ZOBRIST_BLACK;
U64 ZOBRIST_CASTLING[16];
U64 ZOBRIST_EP[8];

void init_zobrist() {
    int i, j, k;
    psrng_u64_seed(0ULL);

    for (i = 0; i < NUM_PIECES; i++) {
        for (j = 0; j < NUM_COLORS; j++) {
            for (k = 0; k < NUM_SQUARES; k++) {
                ZOBRIST_PIECE_SQ[i][j][k] = psrng_u64();
            }
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
    
    U64 byte_size = (U64)mb_size * 1024 * 1024;
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

    if ((entry->key == key) && entry->depth > depth) return;
    //if ((entry->key == key) && (entry->depth > depth || mate_depth(score))) return;
    //if (entry->key != key) return;

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
        for (int j = 0; j < NUM_COLORS; j++) {
            bb = board->pieces[i] & board->colors[j];

            while (bb) {
                sq = LOG2(pop_lsb(&bb));
                hash ^= ZOBRIST_PIECE_SQ[i][j][sq];
            }
        }
    }

    if (board->side_to_move == BLACK)
        hash ^= ZOBRIST_BLACK;

    hash ^= ZOBRIST_CASTLING[(state >> 27) & 0xf];
    if (state & 0x04000000)
        hash ^= ZOBRIST_EP[((state >> 20) & 0x3f) % 8];

    return hash;
}

int mate_depth(int score) {
    if (abs(score) <= CHECKMATE_CP)
        return 0;

    return abs(abs(score) - CHECKMATE_CP - 99);
}

int mate_score(int score) {
    if (abs(score) <= CHECKMATE_CP)
        return 0;

    int depth = mate_depth(score) + 1;
    int mate = depth / 2;
    if (depth % 2 == 0)
        mate *= -1;

    return mate_depth(mate);
}

void print_tt(TTEntry* entry) {
    if (!entry) {
        printf("{ NULL }");
    } else {
        printf("{ %lx - ", entry->key);
        if (entry->type == EMPTY_NODE) {
            printf("EMPTY");
        } else {
            char x = '\0';
            if (entry->type == EXACT_NODE) {
                x = '=';
            } else if (entry->type == ALL_NODE) {
                x = '<';
            } else if (entry->type == ALL_NODE) {
                x = '>';
            } else {
                x = '?';
            }
            printf("x %c %d - depth=%d - ", x, entry->score, entry->depth);
            print_move(entry->best);
        }
        printf(" }");
    }
}

void print_table() {
    int i, j;

    for (i = 0; i < NUM_PIECES; i++) {
        for (j = 0; j < NUM_SQUARES; j++) {
            printf("%lx\n", ZOBRIST_PIECE_SQ[i][0][j]); // whatever
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
