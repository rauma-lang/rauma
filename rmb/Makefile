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
       $(SRC_DIR)/checker.c \
       $(SRC_DIR)/cgen.c \
       $(SRC_DIR)/build.c

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
	python -c "import pathlib; pathlib.Path('$(BUILD_DIR)').mkdir(parents=True, exist_ok=True)"

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
	python -c "import shutil; shutil.rmtree('$(BUILD_DIR)', ignore_errors=True)"

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
	@python -c "import subprocess, sys; rc=subprocess.run(['./$(TARGET)','lex','tests/lexer_errors.rm'], stderr=subprocess.DEVNULL).returncode; print('SUCCESS: lexer_errors.rm failed as expected' if rc else 'ERROR: Expected lexer_errors.rm to fail but it succeeded'); sys.exit(0 if rc else 1)"
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
	@python -c "import subprocess, sys; rc=subprocess.run(['./$(TARGET)','parse','tests/parser_errors.rm'], stderr=subprocess.DEVNULL).returncode; print('SUCCESS: parser_errors.rm failed as expected' if rc else 'ERROR: Expected parser_errors.rm to fail but it succeeded'); sys.exit(0 if rc else 1)"
	@echo ""
	@echo "=== Diagnostics parser test (should fail) ==="
	@python -c "import subprocess, sys; rc=subprocess.run(['./$(TARGET)','parse','tests/parser_diagnostics.rm'], stderr=subprocess.DEVNULL).returncode; print('SUCCESS: parser_diagnostics.rm failed as expected' if rc else 'ERROR: Expected parser_diagnostics.rm to fail but it succeeded'); sys.exit(0 if rc else 1)"
	@echo ""
	@echo "=== Recovery parser test (should fail) ==="
	@python -c "import subprocess, sys; rc=subprocess.run(['./$(TARGET)','parse','tests/parser_recovery.rm'], stderr=subprocess.DEVNULL).returncode; print('SUCCESS: parser_recovery.rm failed as expected (recovery demonstrated)' if rc else 'ERROR: Expected parser_recovery.rm to fail but it succeeded'); sys.exit(0 if rc else 1)"
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
	@python -c "import subprocess, sys; rc=subprocess.run(['./$(TARGET)','check','tests/check_errors.rm'], stderr=subprocess.DEVNULL).returncode; print('SUCCESS: check_errors.rm failed as expected' if rc else 'ERROR: Expected check_errors.rm to fail but it succeeded'); sys.exit(0 if rc else 1)"
	@echo ""
	@echo "=== Build hello test ==="
	@./$(TARGET) build tests/build_hello.rm
	@./$(BUILD_DIR)/debug/native/bin/build_hello
	@echo ""
	@echo "=== Build functions test ==="
	@./$(TARGET) build tests/build_functions.rm
	@./$(BUILD_DIR)/debug/native/bin/build_functions
	@echo ""
	@echo "=== Build if/while test ==="
	@./$(TARGET) build tests/build_if_while.rm
	@./$(BUILD_DIR)/debug/native/bin/build_if_while
	@echo ""
	@echo "=== Build struct basic test ==="
	@./$(TARGET) build tests/build_struct_basic.rm
	@./$(BUILD_DIR)/debug/native/bin/build_struct_basic
	@echo ""
	@echo "=== Build string ops test ==="
	@./$(TARGET) build tests/build_string_ops.rm
	@./$(BUILD_DIR)/debug/native/bin/build_string_ops
	@echo ""
	@echo "=== Build args test ==="
	@./$(TARGET) build tests/build_args.rm
	@./$(BUILD_DIR)/debug/native/bin/build_args
	@./$(BUILD_DIR)/debug/native/bin/build_args version
	@./$(BUILD_DIR)/debug/native/bin/build_args wat
	@echo ""
	@echo "=== Build file read test ==="
	@./$(TARGET) build tests/build_file_read.rm
	@./$(BUILD_DIR)/debug/native/bin/build_file_read
	@echo ""
	@echo "=== Build print slice test ==="
	@./$(TARGET) build tests/build_print_slice.rm
	@./$(BUILD_DIR)/debug/native/bin/build_print_slice
	@echo ""
	@echo "=== Build write_file test ==="
	@./$(TARGET) build tests/build_write_file.rm
	@./$(BUILD_DIR)/debug/native/bin/build_write_file
	@echo ""
	@echo "=== Build cc_compile test ==="
	@./$(TARGET) build tests/build_cc_compile.rm
	@./$(BUILD_DIR)/debug/native/bin/build_cc_compile
	@./$(BUILD_DIR)/rauma_cc_bridge
	@echo ""
	@echo "=== Build errors test (should fail) ==="
	@python -c "import subprocess, sys; rc=subprocess.run(['./$(TARGET)','build','tests/build_errors.rm'], stderr=subprocess.DEVNULL).returncode; print('SUCCESS: build_errors.rm failed as expected' if rc else 'ERROR: Expected build_errors.rm to fail but it succeeded'); sys.exit(0 if rc else 1)"
	@echo ""
	@echo "=== Project basic build test ==="
	@./$(TARGET) build tests/project_basic/main.rm
	@./$(BUILD_DIR)/debug/native/bin/main
	@echo ""
	@echo "=== Project struct build test ==="
	@./$(TARGET) build tests/project_struct/main.rm
	@./$(BUILD_DIR)/debug/native/bin/main
	@echo ""
	@echo "=== Project nested build test ==="
	@./$(TARGET) build tests/project_nested/main.rm
	@./$(BUILD_DIR)/debug/native/bin/main
	@echo ""
	@echo "=== Project missing-use test (should fail) ==="
	@python -c "import subprocess, sys; rc=subprocess.run(['./$(TARGET)','build','tests/project_errors/missing.rm'], stderr=subprocess.DEVNULL).returncode; print('SUCCESS: project_errors/missing.rm failed as expected' if rc else 'ERROR: Expected project_errors/missing.rm to fail but it succeeded'); sys.exit(0 if rc else 1)"
	@$(MAKE) test-rmc-emit-workflow

# External rmc emit-c -> gcc -> run workflow
RMC_BIN = $(BUILD_DIR)/debug/native/bin/main
test-rmc-emit-workflow: debug
	@echo ""
	@echo "=== rmc emit-c external compile workflow ==="
	@./$(TARGET) build ../rmc/main.rm
	@python -c "import subprocess, sys; out=subprocess.check_output(['./$(RMC_BIN)','emit-c','tests/rmc_emit_workflow_hello.rm']); open('$(BUILD_DIR)/rmc_emit_workflow_hello.c','wb').write(out)"
	@$(CC) -std=c11 -Wall -Wextra -Werror -pedantic $(BUILD_DIR)/rmc_emit_workflow_hello.c -o $(BUILD_DIR)/rmc_emit_workflow_hello
	@python -c "import subprocess, sys; out=subprocess.check_output(['./$(BUILD_DIR)/rmc_emit_workflow_hello']).decode().replace('\r\n','\n'); want='hello from rmc\n'; print('SUCCESS: hello workflow output matches' if out==want else 'ERROR: hello got %r want %r' % (out,want)); sys.exit(0 if out==want else 1)"
	@python -c "import subprocess, sys; out=subprocess.check_output(['./$(RMC_BIN)','emit-c','tests/rmc_emit_workflow_add.rm']); open('$(BUILD_DIR)/rmc_emit_workflow_add.c','wb').write(out)"
	@$(CC) -std=c11 -Wall -Wextra -Werror -pedantic $(BUILD_DIR)/rmc_emit_workflow_add.c -o $(BUILD_DIR)/rmc_emit_workflow_add
	@python -c "import subprocess, sys; out=subprocess.check_output(['./$(BUILD_DIR)/rmc_emit_workflow_add']).decode().replace('\r\n','\n'); want='42\n'; print('SUCCESS: add workflow output matches' if out==want else 'ERROR: add got %r want %r' % (out,want)); sys.exit(0 if out==want else 1)"
	@python -c "import subprocess, sys; out=subprocess.check_output(['./$(RMC_BIN)','emit-c','tests/rmc_emit_workflow_fail.rm']).decode(); ok=('check failed' in out); print('SUCCESS: fail workflow rejected as expected' if ok else 'ERROR: fail workflow output: %r' % out); sys.exit(0 if ok else 1)"

self-host-test:
	powershell -NoProfile -ExecutionPolicy Bypass -File ../scripts/release/verify-self-host.ps1

# Show help
help:
	@echo "RauMa Bootstrap Compiler Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all/debug   Build debug version (default)"
	@echo "  release     Build release version"
	@echo "  run         Build and run the compiler"
	@echo "  test        Run lexer tests"
	@echo "  test-rmc-emit-workflow  Verify rmc emit-c -> gcc -> run pipeline"
	@echo "  self-host-test  Verify deterministic real rmc self-host chain"
	@echo "  clean       Remove build artifacts"
	@echo "  help        Show this help"
	@echo ""
	@echo "The executable will be at: $(TARGET)"

.PHONY: all debug release run test clean help test-rmc-emit-workflow self-host-test
