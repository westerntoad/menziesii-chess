#include <time.h>
#include "board.h"
#include "types.h"
#include "eval.h"
#include "movegen.h"
#include "utils.h"

static int TESTS_RUN;
static int TESTS_PASSED;
static bool PERFTS_PASSED;
static U64 PERFT_NODES;

static bool is_move(Move move, Board *board) {
    Move *curr = (Move[256]){0};
    Move *end = legal_moves(board, curr);

    while (curr < end) {
        if (*curr == move)
            return true;

        curr++;
    }
    
    return false;
}

static void assert_move(char* move_str, char* fen, bool is_in, char* message) {
    Board *board = from_fen(fen);
    Move move = move_from_str(board, move_str);

    TESTS_RUN++;

    if (is_move(move, board) != is_in) {
        printf("MOVE ASSERTION FAILED\nFEN       %s\nMOVE      ", fen);
        print_move(move);
        printf("\nMESSAGE   %s\n", message);
        print_board(board);
        print_perft(board, 1);
        printf("\n");
    } else {
        TESTS_PASSED++;
    }

    free(board);
}

static void assert_perft(char* fen, int depth, U64 expected) {
    Board *board = from_fen(fen);
    U64 actual = perft(board, depth);

    TESTS_RUN++;

    if (actual != expected) {
        printf("PERFT ASSERTION FAILED\nFEN       %s\nEXPECTED  %lu\nACTUAL    %lu\n", fen, expected, actual);
        print_board(board);
        printf("\n");
        PERFTS_PASSED = false;
    } else {
        TESTS_PASSED++;
    }

    PERFT_NODES += actual;
    
    free(board);
}

static void assert_piece_eval(char* fen, int expected_eval) {
    Board *board = from_fen(fen);
    int actual = piece_eval(board);
    TESTS_RUN++;
    
    if (expected_eval != actual) {
        printf("PIECE EVAL ASSERTION FAILED\nFEN       %s\nEXPECTED  %d\nACTUAL    %d\n", fen, expected_eval, actual);
    } else {
        TESTS_PASSED++;
    }
    
    free(board);
}

static void assert_eval(char* fen, int depth, int upper_bound, int lower_bound) {
    Board *board = from_fen(fen);
    int actual = eval(board, depth).score;
    TESTS_RUN++;

    if ((lower_bound <= actual) && (actual <= upper_bound)) {
        TESTS_PASSED++;
    } else {
        printf("PIECE EVAL ASSERTION FAILED\nFEN       %s\nUPPER     %d\nLOWER     %d\nACTUAL    %d\n", fen, lower_bound, upper_bound, actual);
    }

    free(board);
}

static void assert_mate(char* fen, int in) {
    Board *board = from_fen(fen);
    PrincipleVariation pv = eval(board, in+1); // TODO remove +1
    TESTS_RUN++;

    if (pv.is_mate && pv.depth != in+1) { // TODO remove +1
        TESTS_PASSED++;
    } else {
        printf("MATE ASSERTION FAILED\nFEN       %s\nMATE_IN   %d\nACTUAL    %d cp\n", fen, in, pv.score);
    }
    
    free(board);
}

static void test_pawns() {
    printf("Testing pawn legal move generation...\n");

    assert_move("g4h3", "7k/8/8/8/6pR/7P/8/K7 b - - 0 1", false, "pawn capture incorrectly adding during check");
    assert_move("f4e3", "7k/8/8/8/4Pp2/8/8/K7 b - e3 0 1", true, "pawn cannot en passant");
    assert_move("f4e3", "8/8/8/3k4/4Pp2/8/8/K7 b - e3 0 1", true, "pawn cannot en passant out of check");
    assert_move("d4e3", "K7/8/8/8/k2pP2R/8/8/8 b - e3 0 1", false, "pawn en passant can cause a self-discovered check");
    assert_move("f4g3", "K7/8/8/6k1/5pP1/8/8/6R1 b - g3 0 1", true, "should be able to en passant pinned pawn");
    assert_move("d7c6", "4k3/3p4/2B5/8/8/8/8/7K b - - 0 1", true, "pawns should be able to capture the piece that pins them");
    assert_move("f4e3", "K7/8/8/6B1/4Pp2/8/8/2k5 b - e3 0 1", true, "pinned pawns can capture ep");
    assert_move("f4g3", "K7/8/8/8/1R3p1k/6P1/8/8 b - - 0 1", false, "pinned pawns cannot capture");
}

static void test_sliders() {
    printf("Testing sliding piece legal move generation...\n");
    
    assert_move("e5g3", "K7/8/4R3/4b3/8/8/8/4k3 b - - 0 1", false, "bishops cannot escape pin");
    assert_move("d2d1", "4k2r/8/8/8/8/2q5/3Q4/r3K2R w - - 0 1", false, "queen cannot leave diagonal slider pin to block check");
    assert_move("d1d2", "4k3/8/8/8/8/2q5/8/q2QK3 w - - 0 1", false, "queen cannot leave orthoganal slider pin to block check");
}

static void test_perfts() {
    clock_t start = clock(), end;
    double duration;
    PERFTS_PASSED = true;
    PERFT_NODES = 0ULL;
    printf("Testing perfts...\n");

    // The following tests were taken from peterellisjones on Github Gist
    // https://gist.github.com/peterellisjones/8c46c28141c162d1d8a0f0badbc9cff9
    assert_perft("r6r/1b2k1bq/8/8/7B/8/8/R3K2R b KQ - 3 2", 1, 8);
    assert_perft("8/8/8/2k5/2pP4/8/B7/4K3 b - d3 0 3", 1, 8);
    assert_perft("r1bqkbnr/pppppppp/n7/8/8/P7/1PPPPPPP/RNBQKBNR w KQkq - 2 2", 1, 19);
    assert_perft("r3k2r/p1pp1pb1/bn2Qnp1/2qPN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQkq - 3 2", 1, 5);
    assert_perft("2kr3r/p1ppqpb1/bn2Qnp1/3PN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQ - 3 2", 1, 44);
    assert_perft("rnb2k1r/pp1Pbppp/2p5/q7/2B5/8/PPPQNnPP/RNB1K2R w KQ - 3 9", 1, 39);
    assert_perft("2r5/3pk3/8/2P5/8/2K5/8/8 w - - 5 4", 1, 9);
    assert_perft("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 3, 62379);
    assert_perft("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 3, 89890);
    assert_perft("3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1", 6, 1134888);
    assert_perft("8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1", 6, 1015133);
    assert_perft("8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1", 6, 1440467);
    assert_perft("5k2/8/8/8/8/8/8/4K2R w K - 0 1", 6, 661072);
    assert_perft("3k4/8/8/8/8/8/8/R3K3 w Q - 0 1", 6, 803711);
    assert_perft("r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1", 4, 1274206);
    assert_perft("r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1", 4, 1720476);
    assert_perft("2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1", 6, 3821001);
    assert_perft("8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1", 5, 1004658);
    assert_perft("4k3/1P6/8/8/8/8/K7/8 w - - 0 1", 6, 217342);
    assert_perft("8/P1k5/K7/8/8/8/8/8 w - - 0 1", 6, 92683);
    assert_perft("K1k5/8/P7/8/8/8/8/8 w - - 0 1", 6, 2217);
    assert_perft("8/k1P5/8/1K6/8/8/8/8 w - - 0 1", 7, 567584);
    assert_perft("8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1", 4, 23527);

    assert_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 5, 4865609);
    assert_perft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 4, 4085603);
    assert_perft("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 5, 674624);
    assert_perft("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 4, 422333);
    assert_perft("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 4, 2103487);
    assert_perft("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 4, 3894594);
    
    end = clock();
    duration = (double)(end - start) / CLOCKS_PER_SEC;

    if (PERFTS_PASSED) {
        printf("All perft tests passed with an average speed of %.2lf million nps\n", (double)((PERFT_NODES / duration) / 1000000));
    }
}

static void test_piece_eval() {
    printf("Testing piece evals...\n");

    assert_piece_eval("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 0);
    assert_piece_eval("8/1k2p3/3p4/8/8/8/2K3P1/8 w - - 0 1", -100);
    assert_piece_eval("8/p7/3R4/3pB3/3Pk1p1/1P3p2/P5r1/5K2 b - - 11 42", 200);
    assert_piece_eval("3k4/7r/6q1/8/2K5/8/8/8 w - - 0 1", -1400);
    assert_piece_eval("rnb1kb1r/ppp1pppp/8/q2n4/8/5N2/PPPP1PPP/R1BQKB1R w KQkq - 0 6", -300);
}

static void test_eval() {
    printf("Testing general evals...\n");
    
    assert_eval("8/p2B2B1/p7/p7/p7/p7/pr6/k6K w - - 0 1", 4, 0, 0);
}

static void test_mates() {
    printf("Testing mates...\n");

    // MATES IN 1
    assert_mate("k7/4R3/4PR2/5P2/8/8/8/7K w - - 0 1", 1);
    assert_mate("k7/8/6b1/8/5b2/4b3/8/7K b - - 0 1", 1);
    assert_mate("8/8/8/2P3R1/5B2/2rP4/pQP1pP2/Rn2K2k w Q - 0 4", 1);
    //assert_mate("r1bqkbnr/1ppp1ppp/p1n5/4p3/2B1P3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 0 4", 1);

    // MATES IN 2
    assert_mate("kbK5/pp6/1P6/8/8/8/8/R7 w - - 0 1", 3);



    //assert_mate("", 0);
}

int main(void) {
    init_move_lookup_tables();
    TESTS_RUN = 0;
    TESTS_PASSED = 0;

    test_pawns();
    test_sliders();
    test_perfts();
    test_piece_eval();
    test_eval();
    test_mates();

    if (TESTS_RUN == TESTS_PASSED) {
        printf("\nAll tests passed.\n");
    } else {
        printf("\n%d/%d tests passed.\n", TESTS_PASSED, TESTS_RUN);
    }
    return 0;
}
