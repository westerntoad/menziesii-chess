#ifndef UTILS_H // include guard
#define UTILS_H

#include <stdio.h>  // For fprintf()
#include <assert.h> // For assert()

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


#endif  // UTILS_H
