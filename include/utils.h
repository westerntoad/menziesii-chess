#ifndef UTILS_H // include guard
#define UTILS_H

#include <stdio.h>
#include <wchar.h>  // used for unicode printing
#include <assert.h>

#ifdef DEBUG
    #define LOG(...) printf(__VA_ARGS__)
    
    #define debug_assert(cond, msg) \
        do { \
            if (!(cond)) { \
                fprintf(stderr, "Assertion failed: %s\nFile: %s, Line: %d\nMessage: %s\n", \
                        #cond, __FILE__, __LINE__, msg); \
                assert(cond); \
            } \
        } while (0)
#else
    #define LOG(...) ((void)0)
    #define debug_assert(cond, msg) ((void)0)
#endif

// https://stackoverflow.com/a/523737
#define TEST_BIT(x,pos) ((x) & (1<<(pos)))
#define SET_BIT(x,pos) ((x) | (1<<(pos)))
#define LOG2(x) __builtin_clzll(x)

#endif  // UTILS_H
