# RauMa Bootstrap Compiler Makefile

# Compiler and flags
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror -pedantic -fno-strict-aliasing
CFLAGS += -Iinclude

# Debug flags (default)
DEBUG_CFLAGS = -g -O0
RELEASE_CFLAGS = -O3 -DNDEBUG

# Build directories
BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include

# Source files
SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/common.c \
       $(SRC_DIR)/arena.c \
       $(SRC_DIR)/string.c \
       $(SRC_DIR)/vec.c \
       $(SRC_DIR)/diag.c \
       $(SRC_DIR)/source.c \
       $(SRC_DIR)/span.c \
       $(SRC_DIR)/token.c \
       $(SRC_DIR)/lexer.c \
       $(SRC_DIR)/ast.c \
       $(SRC_DIR)/parser.c \
       $(SRC_DIR)/type.c \
       $(SRC_DIR)/checker.c

# Object files
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Executable
TARGET = $(BUILD_DIR)/rmb

# Default target
all: debug

# Debug build
debug: CFLAGS += $(DEBUG_CFLAGS)
debug: $(TARGET)

# Release build
release: CFLAGS += $(RELEASE_CFLAGS)
release: $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Run the compiler
run: debug
	./$(TARGET)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Run tests
test: debug
	@echo "Running tests..."
	@echo "=== Version ==="
	@./$(TARGET) version
	@echo ""
	@echo "=== Basic lexer test ==="
	@./$(TARGET) lex tests/lexer_basic.rm
	@echo ""
	@echo "=== String lexer test ==="
	@./$(TARGET) lex tests/lexer_strings.rm
	@echo ""
	@echo "=== Error lexer test (should fail) ==="
	@if ./$(TARGET) lex tests/lexer_errors.rm 2>/dev/null; then \
		echo "ERROR: Expected lexer_errors.rm to fail but it succeeded"; \
		exit 1; \
	else \
		echo "SUCCESS: lexer_errors.rm failed as expected"; \
	fi
	@echo ""
	@echo "=== Basic parser test ==="
	@./$(TARGET) parse tests/parser_basic.rm
	@echo ""
	@echo "=== Struct/Enum parser test ==="
	@./$(TARGET) parse tests/parser_struct_enum.rm
	@echo ""
	@echo "=== Match parser test ==="
	@./$(TARGET) parse tests/parser_match.rm
	@echo ""
	@echo "=== Types parser test ==="
	@./$(TARGET) parse tests/parser_types.rm
	@echo ""
	@echo "=== Error parser test (should fail) ==="
	@if ./$(TARGET) parse tests/parser_errors.rm 2>/dev/null; then \
		echo "ERROR: Expected parser_errors.rm to fail but it succeeded"; \
		exit 1; \
	else \
		echo "SUCCESS: parser_errors.rm failed as expected"; \
	fi
	@echo ""
	@echo "=== Diagnostics parser test (should fail) ==="
	@if ./$(TARGET) parse tests/parser_diagnostics.rm 2>/dev/null; then \
		echo "ERROR: Expected parser_diagnostics.rm to fail but it succeeded"; \
		exit 1; \
	else \
		echo "SUCCESS: parser_diagnostics.rm failed as expected"; \
	fi
	@echo ""
	@echo "=== Recovery parser test (should fail) ==="
	@if ./$(TARGET) parse tests/parser_recovery.rm 2>/dev/null; then \
		echo "ERROR: Expected parser_recovery.rm to fail but it succeeded"; \
		exit 1; \
	else \
		echo "SUCCESS: parser_recovery.rm failed as expected (recovery demonstrated)"; \
	fi
	@echo ""
	@echo "=== Check basic test ==="
	@./$(TARGET) check tests/check_basic.rm
	@echo ""
	@echo "=== Check functions test ==="
	@./$(TARGET) check tests/check_functions.rm
	@echo ""
	@echo "=== Check optionals test ==="
	@./$(TARGET) check tests/check_optionals.rm
	@echo ""
	@echo "=== Check error flow test ==="
	@./$(TARGET) check tests/check_error_flow.rm
	@echo ""
	@echo "=== Check errors test (should fail) ==="
	@if ./$(TARGET) check tests/check_errors.rm 2>/dev/null; then \
		echo "ERROR: Expected check_errors.rm to fail but it succeeded"; \
		exit 1; \
	else \
		echo "SUCCESS: check_errors.rm failed as expected"; \
	fi

# Show help
help:
	@echo "RauMa Bootstrap Compiler Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all/debug   Build debug version (default)"
	@echo "  release     Build release version"
	@echo "  run         Build and run the compiler"
	@echo "  test        Run lexer tests"
	@echo "  clean       Remove build artifacts"
	@echo "  help        Show this help"
	@echo ""
	@echo "The executable will be at: $(TARGET)"

.PHONY: all debug release run test clean help