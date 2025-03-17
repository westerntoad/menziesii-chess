#include <stdio.h>
#include "types.h"
#include "movegen.h"

int main() {
    init_n_moves();
    U64 e4 = 0x0000000000000040ULL;
    printf("\n");
    print_bb(e4);
    e4 = n_moves(e4);
    printf("n_moves\n");
    print_bb(e4);
}
