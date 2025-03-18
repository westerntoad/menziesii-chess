# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2
DEBUG_CFLAGS = -DDEBUG -g # Debug flag
LDFLAGS = -lm  # Link math library (if needed)

# Directories
SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = obj
BIN_DIR = bin
TEST_DIR = tests

# Source and object files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Executables
EXEC = $(BIN_DIR)/menziesii
TEST_EXEC = $(BIN_DIR)/test_menziesii

# Target to build the main chessbot executable
all: $(BIN_DIR) $(OBJ_DIR) $(EXEC)

# Create the bin and obj directories if they don't exist
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# Main Chess Bot executable
$(EXEC): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $(EXEC) $(LDFLAGS)

# Object files compilation rule
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Target for unit tests
tests: $(TEST_EXEC)

$(TEST_EXEC): $(OBJ_FILES) $(TEST_DIR)/test_chessbot.o
	$(CC) $(OBJ_FILES) $(TEST_DIR)/test_chessbot.o -o $(TEST_EXEC) $(LDFLAGS)

# Test object files compilation
$(TEST_DIR)/test_chessbot.o: $(TEST_DIR)/test_chessbot.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Clean object and binary files
clean:
	rm -rf $(OBJ_DIR)/*.o $(EXEC) $(TEST_EXEC)

# Debug build (with debug symbols)
debug: CFLAGS += $(DEBUG_CFLAGS)
debug: clean all

# Run unit tests
run: $(EXEC)
	@./$(EXEC)

# Run unit tests
run-tests: tests
	./$(TEST_EXEC)

run-debug: debug
	./$(EXEC)

# Generate dependency files for automatic rebuild
deps: $(OBJ_FILES:.o=.d)

# Include dependency files
-include $(OBJ_FILES:.o=.d)

# Phony targets (not associated with files)
.PHONY: all clean debug tests run run-tests deps
