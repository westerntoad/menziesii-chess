#include "utils.h"

// https://en.wikipedia.org/wiki/Xorshift#Example_implementation
static U64 XORSHIFT_STATE_64 = 0x445186dc4c335adcULL;

int psrng_u64_seed(U64 seed) {
    if (seed) {
        XORSHIFT_STATE_64 = seed;
        return 1;
    } else {
        XORSHIFT_STATE_64 = 0x445186dc4c335adcULL;
        return 0;
    }
}

U64 psrng_u64() {
    U64 x = XORSHIFT_STATE_64;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return XORSHIFT_STATE_64 = x;
}
