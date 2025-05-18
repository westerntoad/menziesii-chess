#include <time.h>
#include "board.h"
#include "table.h"
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

static void assert_eval(char* fen, int depth, int upper_bound, int lower_bound) {
    Board *board = from_fen(fen);
    eval(board, depth);
    TTEntry* entry = tt_probe(get_hash(board));
    int actual = upper_bound;
    TESTS_RUN++;
    if (!entry) {
        printf("ERROR READING ENTRY IN EVAL ASSERTION\n");
        TESTS_PASSED--;
    } else {
        actual = entry->score;
    }

    if ((lower_bound <= actual) && (actual <= upper_bound)) {
        TESTS_PASSED++;
    } else {
        printf("EVAL ASSERTION FAILED\nFEN       %s\nUPPER     %d\nLOWER     %d\nACTUAL    %d\n", fen, lower_bound, upper_bound, actual);
    }

    free(board);
}

static void assert_mate(char* fen, int in) {
    Board *board = from_fen(fen);
    eval(board, in+1);
    TTEntry* entry = tt_probe(get_hash(board));
    TESTS_RUN++;

    if (abs(entry->score) > CHECKMATE_CP && entry->depth != in) {
        TESTS_PASSED++;
    } else {
        if (abs(entry->score) <= CHECKMATE_CP) {
            printf("MATE ASSERTION FAILED - NOT MATE\nFEN       %s\nACTUAL    %d cp\nDEPTH     %d\n", fen, entry->score, entry->depth);
        } else {
            printf("MATE ASSERTION FAILED - INCORRECT DEPTH\nFEN       %s\nEXPECTED  %d\nACTUAL    %d\n", fen, in, entry->depth);
        }

    }
    
    free(board);
}

static void assert_procedural_hashing(char* fen, Move move) {
    Board *board = from_fen(fen);
    U64 actual, expected = get_hash(board);
    bool passed = true;
    TESTS_RUN++;

    if (move) {
        make_move(board, move);
        unmake_move(board, move);

        actual = get_hash(board);

        passed = expected == get_hash(board);
    } else {
        Move *curr = (Move[256]){0};
        Move *end = legal_moves(board, curr);
        if (curr == end)
            return;

        while (curr != end && passed) {
            make_move(board, *curr);
            unmake_move(board, *curr);
            actual = get_hash(board);

            if (expected != actual) {
                move = *curr;
                passed = false;
                break;
            }
            
            curr++;
        }
    }

    if (passed) {
        TESTS_PASSED++;
    } else {
        printf("PROCEDURAL ZOBRIST HASHING ASSERTION FAILED\nFEN       %s\nMOVE      ", fen);
        print_move(move);
        printf("\nEXPECTED  %lx\nACTUAL    %lx\n", expected, actual);
    }

    free(board);
}

static void assert_unequal_hashes(char* fen1, char* fen2) {
    Board *board1 = from_fen(fen1);
    Board *board2 = from_fen(fen2);
    TESTS_RUN++;

    if (get_hash(board1) != get_hash(board2)) {
        TESTS_PASSED++;
    } else {
        printf("PROCEDURAL ZOBRIST HASHING ASSERTION FAILED - HASH COLLISION OF TWO UNEQUAL FENS\nFEN1      %s\nFEN2      %s\n", fen1, fen2);
    }

    free(board1);
    free(board2);
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
    assert_perft("4B3/5p2/4P1k1/8/8/8/8/K7 b - - 0 3", 1, 7);
    
    end = clock();
    duration = (double)(end - start) / CLOCKS_PER_SEC;

    if (PERFTS_PASSED) {
        printf("All perft tests passed with an average speed of %.2lf million nps\n", (double)((PERFT_NODES / duration) / 1000000));
    }
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
    assert_mate("r1bqkbnr/1ppp1ppp/p1n5/4p3/2B1P3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 0 4", 1);

    // MATES IN 2
    assert_mate("kbK5/pp6/1P6/8/8/8/8/R7 w - - 0 1", 3);
    assert_mate("8/8/8/2P3R1/5B2/2rP1p2/p1P1PP2/RnQ1K2k w Q - 5 3", 3);
    assert_mate("8/8/2Q5/3B4/1K6/2P5/Nk6/2R5 w - - 0 1", 4);
    assert_mate("5B2/8/K7/8/kpp5/7R/8/1B6 w - - 0 1", 4);
    //assert_mate("", 4);

    //assert_mate("", 0);
}

static void test_state_stack() {
    printf("Stress-testing state stack...\n");
    int i, j;
    int total_tests = 64;
    int num_kk = 0;

    for (i = 0; i < total_tests; i++) {
        Board* board = from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        Move random = random_move(board);
        for (j = 0; j < (2<<16) && random != 0; j++) {
            //printf("%d\n", i);
            make_move(board, random);
            random = random_move(board);
        }
        if (j == (2<<16))
            num_kk++;
    }

    printf("Random matches had a %.1lf%% rate of being a king-king endamge (n=%d)\n", ((double)num_kk) / ((double)total_tests)*100, total_tests);
}

static void test_procedural_hashing() {
    printf("Testing procedural hashing...\n");
    
    assert_procedural_hashing("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 0); // starting
    assert_procedural_hashing("r1bqk2r/pppp1ppp/2n2n2/2b1p3/2B1P3/2N2N2/PPPP1PPP/R1BQK2R w KQkq - 6 5", 0); // castling
    assert_procedural_hashing("r1bqkbnr/ppp1pppp/2n5/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3", 0); // en passant
    
    assert_unequal_hashes("4k3/8/8/8/8/1r6/8/3R1K2 w - - 4 3", "4k3/8/8/8/8/1R6/8/3r1K2 w - - 4 3");
    assert_unequal_hashes("rnbqkbnr/p1pppppp/8/8/p1P5/8/1P1PPPPP/RNBQKBNR b KQkq - 0 3", "rnbqkbnr/p1pppppp/8/8/p1p5/8/1P1PPPPP/RNBQKBNR b KQkq - 0 1");
}

static void test_draws() {
    printf("Testing threefold repitition...\n");
    
    int passed = true;
    char* fen = "r1b1r3/ppp3kp/2n5/3p1pp1/3P4/B1P3KP/P1P3P1/4R3 w - - 0 22";
    Board* board = from_fen(fen);
    TESTS_RUN++;
    make_move(board, move_from_str(board, "e1e8"));
    passed = passed && !is_threefold(board);
    make_move(board, move_from_str(board, "g7f7"));
    passed = passed && !is_threefold(board);
    make_move(board, move_from_str(board, "e8f8"));
    passed = passed && !is_threefold(board);
    make_move(board, move_from_str(board, "f7g7"));
    passed = passed && !is_threefold(board);
    make_move(board, move_from_str(board, "f8e8"));
    passed = passed && !is_threefold(board);
    make_move(board, move_from_str(board, "g7f7"));
    passed = passed && !is_threefold(board);
    make_move(board, move_from_str(board, "e8f8"));
    passed = passed && !is_threefold(board);
    make_move(board, move_from_str(board, "f7g7"));
    passed = passed && !is_threefold(board);
    make_move(board, move_from_str(board, "f8e8"));
    //passed = passed && is_threefold(board);
    if (!passed) {
        printf("THREEFOLD REPTITION NOT DETECTED FOR GAME\nFEN       %s\n", fen);
    } else {
        TESTS_PASSED++;
    }
}


int main(void) {
    srand(time(NULL));
    setbuf(stdout, NULL);
    init_move_lookup_tables();
    init_zobrist();
    tt_set_size(256);
    TESTS_RUN = 0;
    TESTS_PASSED = 0;

    test_pawns();
    test_sliders();
    test_perfts();
    test_eval();
    test_mates();
    test_state_stack();
    test_procedural_hashing();
    test_draws();

    if (TESTS_RUN == TESTS_PASSED) {
        printf("\nAll tests passed.\n");
    } else {
        printf("\n%d/%d tests passed.\n", TESTS_PASSED, TESTS_RUN);
    }
    return 0;
}
