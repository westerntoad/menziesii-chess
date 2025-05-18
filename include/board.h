#ifndef BOARD_H // include guard
#define BOARD_H

#include "types.h"

#define MAX_NUM_MOVES 17_697
#define MAX_NUM_LEGAL_MOVES 218

typedef struct {
    U64 colors[NUM_COLORS];
    U64 pieces[NUM_PIECES];
    int ply;
    bool side_to_move;
    int stack_capacity;
    int ply_offset;
    StateFlags *state_stack;
    U64 *hash_stack;
} Board;

void make_move(Board *board, Move move);
void unmake_move(Board *board, Move move);
Move* legal_moves(Board *board, Move *list);
bool is_in_check(Board *board);
U64 get_hash(Board *board);
Move random_move(Board *board);
Move move_from_str(Board *board, char* str);
U64 perft(Board *board, int depth);
void print_perft(Board *board, int depth);
Board* from_fen(char *fen);
char* to_fen(Board *board);
void free_board(Board *board);
Board* copy_board(Board *board);
void print_board(Board *board);
void print_board_bb(Board *board);
void wprint_board(Board *board);
/*void print_move_buffer(Move *buffer);
void wprint_move_buffer(Move *buffer);*/

#endif  // BOARD_H
