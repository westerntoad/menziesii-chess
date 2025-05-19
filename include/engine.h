#ifndef ENGINE_H // include guard
#define ENGINE_H

#include "types.h"

#define DEFAULT_TT_SIZE 64

typedef struct {
    int infinite;
    U8  depth;
    int movetime;
    int wtime;
    int btime;
    int winc;
    int binc;
    int print_info;
} SearchParams;

static const SearchParams PARAMS_DEFAULT = (SearchParams){
    .infinite = 0,
    .depth = 99,
    .wtime = 0,
    .btime = 0,
    .winc = 0,
    .binc = 0,
    .print_info = 1
};

void engine_init();
void engine_set_debug(bool mode);
bool engine_is_debug();
void engine_move(char* move_str);
void engine_unmove();
void engine_quit();
void resize_engine_table(int mb_size);
int set_position(char* fen, char** moves);
void print_engine();
void go_perft(int depth);
void go_random();
void start_search(SearchParams params);
int stop_search();


#endif  // ENGINE_H
