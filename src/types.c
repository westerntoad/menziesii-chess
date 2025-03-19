#include "types.h"
#include "utils.h" // includes <stdio.h>

void print_sq(Sq sq) {
    wprintf(L"%c%d", 0x61 + (sq%8), sq/8 + 1);
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

void print_move(Move move) {
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
            wprintf(L"%lc ", ((bb >> (j+i*8)) & 1ULL) ? 0x2715 : 0x00b7);
        wprintf(L"\n");
    }
}

