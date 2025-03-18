#ifndef TYPES_H // include guard
#define TYPES_H

#include <stdint.h>

typedef uint64_t U64;

#define NUM_SQUARES 64
#define A_FILE 0x0101010101010101ULL 
#define H_FILE 0x8080808080808080ULL 
//const U64 A_FILE = 0x0101010101010101ULL;
//const U64 H_FILE = 0x8080808080808080ULL;

void print_bb(U64 bb);
U64 nort_one(U64 bb);
U64 east_one(U64 bb);
U64 sout_one(U64 bb);
U64 west_one(U64 bb);
U64 delta_one(U64 bb, int dx, int dy);

#endif  // TYPES_H
