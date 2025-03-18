#include <stdio.h>
#include "types.h"
#include "movegen.h"

int main() {
    U64 e4, nm, km, rm, bm, qm;
    init_move_lookup_tables();
    e4 = 0x0000000010000000ULL;
    //rm = 0ULL;
    print_bb(e4);
    printf("n_moves\n");
    nm = n_moves(e4);
    print_bb(nm);
    printf("k_moves\n");
    km = k_moves(e4);
    print_bb(km);
    printf("r_moves\n");
    rm = r_moves(e4, 0x1000000004000000ULL);
    print_bb(rm);
    printf("b_moves\n");
    bm = b_moves(e4, 0x0100400000000000ULL);
    print_bb(bm);
    printf("q_moves\n");
    qm = q_moves(e4, 0x1100400004000000ULL);
    print_bb(qm);

    /*U64 temp = delta_one(e4, 1, 0);
    printf("delta_one\n");
    print_bb(temp);
    temp = delta_one(temp, 1, 0);
    printf("delta_one\n");
    print_bb(temp);
    temp = delta_one(temp, 1, 0);
    printf("delta_one\n");
    print_bb(temp);
    temp = delta_one(temp, 1, 0);
    printf("delta_one\n");
    print_bb(temp);*/
}
