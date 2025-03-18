#include "types.h"
#include "utils.h" // includes <stdio.h>

void print_bb(U64 bb) {
    int i, j;
    for (i = 7; i >= 0; i--) {
        for (j = 0; j < 8; j++)
            wprintf(L"%llu ", (bb >> (j+i*8)) & 1ULL);
        wprintf(L"\n");
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
