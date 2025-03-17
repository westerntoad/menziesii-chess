
#include <stdio.h>
#include "types.h"

void print_bb(U64 bb) {
    int i, j;
    for (i = 7; i >= 0; i--) {
        for (j = 0; j < 8; j++)
            printf("%llu ", (bb >> (j+i*8)) & 1ULL);
        printf("\n");
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
