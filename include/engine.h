#ifndef ENGINE_H // include guard
#define ENGINE_H

typedef struct {
    int infinite;
    int depth;
    int movetime;
    int wtime;
    int btime;
    int winc;
    int binc;
    int print_info;
} SearchParams;

static const SearchParams PARAMS_DEFAULT = (SearchParams){
    .infinite = 0,
    .depth = 0,
    .wtime = 0,
    .btime = 0,
    .winc = 0,
    .binc = 0,
    .print_info = 1
};

void engine_init();
void start_search(Board *board, SearchParams params); // TODO remove board
int stop_search();


#endif  // ENGINE_H
