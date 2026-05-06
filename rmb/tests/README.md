# RauMa Bootstrap Compiler Tests

## Overview

This directory contains tests for the RauMa Bootstrap Compiler (`rmb`). The tests verify the correctness and stability of the bootstrap compiler.

## Test Categories

### Unit Tests
- **Tokenizer Tests**: Verify tokenization of RauMa source code
- **Parser Tests**: Verify AST construction from tokens
- **Type Checker Tests**: Verify type inference and checking
- **Codegen Tests**: Verify C code generation
- **Utility Tests**: Test arena, string, vector, and diagnostic utilities

### Integration Tests
- **End-to-End Tests**: Full compilation pipeline tests
- **Error Recovery Tests**: Test error handling and reporting
- **Regression Tests**: Historical bug fixes

### Property Tests
- **Random Program Tests**: Generate random valid programs and verify compilation
- **Fuzz Tests**: Test edge cases and invalid inputs

## Test Organization

### Directory Structure
```
tests/
├── unit/                    # Unit tests
│   ├── tokenizer/          # Tokenizer tests
│   ├── parser/             # Parser tests
│   ├── typecheck/          # Type checker tests
│   ├── codegen/            # Code generation tests
│   └── utils/              # Utility tests
├── integration/            # Integration tests
│   ├── e2e/               # End-to-end tests
│   ├── errors/            # Error handling tests
│   └── regressions/       # Regression tests
├── property/              # Property tests
│   ├── random/           # Random program tests
│   └── fuzz/             # Fuzz tests
└── data/                 # Test data files
    ├── valid/           # Valid RauMa programs
    ├── invalid/         # Invalid RauMa programs
    └── expected/        # Expected output files
```

### Test File Format
Each test file follows a consistent format:
```rauma
// test_name.rm
// Description: What this test verifies
// Expected: What should happen

// Test code here
fn test_function() {
    // ...
}
```

## Running Tests

### Prerequisites
- Built `rmb` compiler
- C compiler (gcc/clang)
- Make

### Basic Test Commands
```bash
# Build and run all tests
make test

# Run specific test category
make test-unit
make test-integration
make test-property

# Run individual test
make test TOKENIZER

# Generate test coverage report
make coverage
```

### Manual Testing
```bash
# Test tokenizer
./build/rmb check tests/data/valid/simple.rm

# Test full compilation
./build/rmb build tests/data/valid/hello.rm
./build/hello
```

## Writing Tests

### Unit Test Example
```c
// test_tokenizer.c
#include "rmb/tokenizer.h"
#include "test_utils.h"

TEST(tokenizer_basic) {
    const char* source = "fn main() { return 42; }";
    rmb_tokenizer* tok = rmb_tokenizer_create(source);
    
    TEST_ASSERT_NOT_NULL(tok);
    
    rmb_token token;
    rmb_tokenizer_next(tok, &token);
    TEST_ASSERT_EQ(token.type, RMB_TOKEN_FN);
    
    rmb_tokenizer_destroy(tok);
}
```

### Integration Test Example
```rauma
// test_hello.rm
// Description: Basic hello world compilation
// Expected: Compiles and runs successfully

fn main() {
    print("Hello, world!\n");
}
```

### Property Test Example
```c
// test_random_programs.c
TEST(random_program_compilation) {
    for (int i = 0; i < 1000; i++) {
        char* program = generate_random_program();
        
        // Test that valid programs compile
        if (is_valid_program(program)) {
            TEST_ASSERT(compile_program(program));
        }
        
        free(program);
    }
}
```

## Test Data Management

### Valid Programs
- Simple, correct RauMa programs
- Test specific language features
- Verify successful compilation

### Invalid Programs
- Programs with syntax errors
- Programs with type errors
- Programs with semantic errors
- Verify error detection and reporting

### Expected Output
- Expected compiler output
- Expected error messages
- Expected generated C code

## Continuous Integration

### Automated Testing
- Run tests on every commit
- Test on multiple platforms
- Test with multiple C compilers
- Performance regression testing

### Test Coverage
- Line coverage
- Branch coverage
- Function coverage
- Path coverage for critical code

## Test Maintenance

### Adding New Tests
1. Determine test category (unit/integration/property)
2. Create test file in appropriate directory
3. Add test to build system
4. Verify test passes
5. Update documentation if needed

### Updating Tests
1. Update test when compiler behavior changes
2. Maintain backward compatibility for stable features
3. Document why test was updated

### Removing Tests
1. Only remove tests for removed features
2. Document removal in changelog
3. Update test counts in documentation

## Future Test Enhancements

### Planned Features
- **Parallel Test Execution**: Run tests in parallel
- **Test Isolation**: Each test runs in isolated environment
- **Golden File Testing**: Compare output with expected files
- **Performance Testing**: Benchmark compiler performance
- **Memory Testing**: Detect memory leaks and corruption

### Integration with Main Compiler
- Share test suites between `rmb` and `rmc`
- Verify consistent behavior across compilers
- Support self-hosting test verification