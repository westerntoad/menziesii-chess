#include "types.h"
#include "utils.h" // includes <stdio.h>

#define INITIAL_MOVELIST_CAPACITY 25

Sq flip_v(Sq sq) {
    return (sq % 8) + (7 - (sq / 8)) * 8;
}

void print_sq(Sq sq) {
    printf("%c%d", 0x61 + (sq%8), sq/8 + 1);
}

void wprint_sq(Sq sq) {
    wprintf(L"%c%d", 0x61 + (sq%8), sq/8 + 1);
}

Sq sq_from_str(char *s) {
    return s[0] - 0x61 + (s[1] - 0x31)*8;
}

Move new_move(Sq from, Sq to, MoveFlags flags) {
    return (flags<<12) | (from<<6) | to;
}

Sq get_from(Move move) {
    return (move >> 6) & 0x3f;
}

Sq get_to(Move move) {
    return move & 0x3f;
}

bool is_promotion(Move move) {
    return move & 0x8000;
}

bool is_capture(Move move) {
    return ((move>>12) & 0b1110) == 0b0100;
}

void print_move(Move move) {
    if (move == 0) {
        printf("0000");
        return;
    }

    print_sq(get_from(move));
    print_sq(get_to(move));
    
    if (is_promotion(move)) {
        switch ((move >> 12) & 0x03) {
            case 0x00:
                printf("n");
                break;
            case 0x01:
                printf("b");
                break;
            case 0x02:
                printf("r");
                break;
            case 0x03:
                printf("q");
                break;
        }
    }
}

void wprint_move(Move move) {
    if (((move >> 12) & 0b1110) == 0b10) {
        wprintf(L"O-O");
        if ((move>>12) & 1) { // if is long castle
            wprintf(L"-O");
        }

        return;
    }

    print_sq(get_from(move));
    print_sq(get_to(move));
    
    if (is_promotion(move)) {
        switch ((move >> 12) & 0x03) {
            case 0x00:
                wprintf(L"n");
                break;
            case 0x01:
                wprintf(L"b");
                break;
            case 0x02:
                wprintf(L"r");
                break;
            case 0x03:
                wprintf(L"q");
                break;
        }
    }
}

MoveList *new_movelist() {
    MoveList *list = (MoveList*)malloc(sizeof(MoveList));

    if (list == NULL)
        return NULL;

    list->capacity = INITIAL_MOVELIST_CAPACITY;
    list->size = 0;
    list->data = (Move*)malloc(sizeof(Move) * list->capacity);
    if (list->data == NULL) {
        free(list);
        return NULL;
    }

    return list;
}

void free_movelist(MoveList *list) {
    free(list->data);
    free(list);
}

int push_move(MoveList *list, Move move) {
    if (list->size == list->capacity) {
        list->capacity *= 2;
        list->data = (Move*)realloc(list->data, sizeof(Move) * list->capacity);
        if (list->data == NULL) {
            free(list);
            // TEMPORARY CODE
            fprintf(stderr, "MOVE LIST RAN OUT OF SPACE\n");
            exit(EXIT_FAILURE);
            // TEMPORARY CODE

            return 0;
        }
    }

    list->data[list->size++] = move;

    return 1;
}

Move pop_move(MoveList *list) {
    if (list->size == 0) {
        return 0;
    }

    return list->data[--list->size];
}

void print_movelist(MoveList *list) {
    size_t i;

    for (i = 0; i < list->size; i++) {
        print_move(list->data[i]);
        printf("\n");
    }
}

U64 pop_lsb(U64 *bb) {
    U64 old = *bb;
    *bb &= *bb - 1;
    return old & ~(*bb);
}

U64 nort_one(U64 bb) {
    return bb << 8;
}

U64 east_one(U64 bb) {
    return (bb << 1) & ~A_FILE;
}

U64 sout_one(U64 bb) {
    return bb >> 8;
}

U64 west_one(U64 bb) {
    return (bb >> 1) & ~H_FILE;
}

U64 delta_one(U64 bb, int dx, int dy) {
    debug_assert(dx > -2 && dx < 2, "delta x must be in interval [1, -1]");
    debug_assert(dy > -2 && dy < 2, "delta y must be in interval [1, -1]");
    U64 dbb;

    switch (dx) {
        case  1:
            dbb = east_one(bb);
            break;
        case -1:
            dbb = west_one(bb);
            break;
        default:
            dbb = bb;
    }

    switch (dy) {
        case  1:
            dbb = nort_one(dbb);
            break;
        case -1:
            dbb = sout_one(dbb);
            break;
    }

    return dbb;
}

void print_bb(U64 bb) {
    int i, j;
    for (i = 7; i >= 0; i--) {
        for (j = 0; j < 8; j++)
            printf("%d ", (int)((bb >> (j+i*8)) & 1ULL));
        printf("\n");
    }
}

void wprint_bb(U64 bb) {
    int i, j;
    for (i = 7; i >= 0; i--) {
        for (j = 0; j < 8; j++)
            wprintf(L"%lc ", ((bb >> (j+i*8)) & 1ULL) ? 0x2715 : 0x00b7);
        wprintf(L"\n");
    }
}

