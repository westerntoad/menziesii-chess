#include "board.h"
#include "types.h"
#include "movegen.h"
#include "utils.h"

static int TESTS_RUN;
static int TESTS_PASSED;

static bool is_move(Move move, Board *board) {
    MoveList *list = legal_moves(board);
    size_t i;

    for (i = 0; i < list->size; i++) {
        if (list->data[i] == move) {
            free(list);
            return true;
        }
    }
    
    free(list);
    return false;
}

static void move_assert(char* move_str, char* fen, bool is_in, char* message) {
    Board *board = from_fen(fen);
    Move move = move_from_str(board, move_str);

    TESTS_RUN++;

    if (is_move(move, board) != is_in) {
        printf("ASSERTION FAILED ON MOVE: ");
        print_move(move);
        printf("\n%s\n", message);
        print_board(board);
        print_perft(board, 1);
        printf("\n");
    } else {
        TESTS_PASSED++;
    }

    free(board);
}

static void test_pawns() {
    printf("Testing pawn legal move generation...\n");

    move_assert("g4h3", "7k/8/8/8/6pR/7P/8/K7 b - - 0 1", false, "pawn capture incorrectly adding during check");
    move_assert("f4e3", "7k/8/8/8/4Pp2/8/8/K7 b - e3 0 1", true, "pawn cannot en passant");
    move_assert("f4e3", "8/8/8/3k4/4Pp2/8/8/K7 b - e3 0 1", true, "pawn cannot en passant out of check");
    move_assert("d4e3", "K7/8/8/8/k2pP2R/8/8/8 b - e3 0 1", false, "pawn en passant can cause a self-discovered check");
    move_assert("f4g3", "K7/8/8/6k1/5pP1/8/8/6R1 b - g3 0 1", true, "should be able to en passant pinned pawn");
    move_assert("d7c6", "4k3/3p4/2B5/8/8/8/8/7K b - - 0 1", true, "pawns should be able to capture the piece that pins them");
    move_assert("f4e3", "K7/8/8/6B1/4Pp2/8/8/2k5 b - e3 0 1", true, "pinned pawns can capture ep");
}

static void test_sliders() {
    printf("Testing pawn legal move generation...\n");
    
    move_assert("e5g3", "K7/8/4R3/4b3/8/8/8/4k3 b - - 0 1", false, "bishops cannot escape pin");
}

int main(void) {
    init_move_lookup_tables();
    TESTS_RUN = 0;
    TESTS_PASSED = 0;

    /*Board *board = from_fen("8/8/8/8/k2Pp2R/8/8/K7 b - - 0 1");
    print_board(board);
    print_perft(board, 1);
    return 0;*/

    test_pawns();
    test_sliders();

    if (TESTS_RUN == TESTS_PASSED) {
        printf("\nAll tests passed.\n");
    } else {
        printf("\n%d/%d tests passed.\n", TESTS_PASSED, TESTS_RUN);
    }
    return 0;
}
