# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2
BUILD_DATE := $(shell git log -1 --format=%cd --date=format:%Y%m%d)
GIT_HASH := $(shell git rev-parse --short HEAD)
CFLAGS += -DBUILD_DATE=$(BUILD_DATE) -DGIT_HASH=\"$(GIT_HASH)\"
DEBUG_CFLAGS = -DDEBUG -g
LDFLAGS = -lm

# Directories
SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = obj
BIN_DIR = bin
TEST_DIR = tests

# Source and object files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(filter-out $(SRC_DIR)/main.c, $(SRC_FILES))
OBJ_FILES := $(OBJ_FILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
MAIN_OBJ = $(OBJ_DIR)/main.o

# Executables
EXEC = $(BIN_DIR)/menziesii
TEST_EXEC = $(BIN_DIR)/test_menziesii

# Target to build the main chessbot executable
all: $(EXEC)

# Create directories if they don't exist
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# Main executable
$(EXEC): $(OBJ_FILES) $(MAIN_OBJ) | $(BIN_DIR)
	$(CC) $(OBJ_FILES) $(MAIN_OBJ) -o $(EXEC) $(LDFLAGS)

# Object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Compile main separately
$(MAIN_OBJ): $(SRC_DIR)/main.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Unit tests
tests: CFLAGS += $(DEBUG_CFLAGS)
tests: $(TEST_EXEC)

$(TEST_EXEC): $(OBJ_FILES) $(TEST_DIR)/test_chessbot.o | $(BIN_DIR)
	$(CC) $(OBJ_FILES) $(TEST_DIR)/test_chessbot.o -o $(TEST_EXEC) $(LDFLAGS)

# Test object file
$(TEST_DIR)/test_chessbot.o: $(TEST_DIR)/test_menziesii.c $(wildcard $(INCLUDE_DIR)/*.h)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Clean
clean:
	rm -rf $(OBJ_DIR)/*.o $(EXEC) $(TEST_EXEC)

# Debug build
debug: CFLAGS += $(DEBUG_CFLAGS)
debug: clean all

# Run targets
run: $(EXEC)
	@./$(EXEC)

run-tests: tests
	./$(TEST_EXEC)

run-debug: debug
	./$(EXEC)

# Dependencies
deps: $(OBJ_FILES:.o=.d)

-include $(OBJ_FILES:.o=.d)

.PHONY: all clean debug tests run run-tests deps

