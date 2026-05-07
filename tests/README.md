# RauMa Compiler Tests

## Overview

This directory contains comprehensive tests for the RauMa compiler. Tests are organized by feature and complexity to ensure compiler correctness, stability, and performance.

## Test Organization

### By Feature
```
tests/
├── syntax/           # Syntax parsing tests
├── parse/            # Full parser tests
├── type/             # Type checking tests
├── error/            # Error handling tests
├── interference/     # Interference checking tests
├── codegen/          # Code generation tests
└── selfhost/         # Self-hosting verification tests
```

### By Complexity
- **Unit Tests**: Individual compiler components
- **Integration Tests**: Multi-component interactions
- **End-to-End Tests**: Full compilation pipeline
- **Regression Tests**: Historical bug fixes
- **Performance Tests**: Compilation speed and memory usage

## Test Categories

### Syntax Tests (`tests/syntax/`)
Test the lexer/tokenizer and basic syntax parsing.

**Example Tests:**
- Valid token sequences
- Invalid token sequences
- Keyword recognition
- Operator parsing
- Comment handling
- String/character literals
- Number literals (decimal, hex, binary)

**Example:**
```rauma
// test_comments.rm
// Should parse successfully with comments

fn main() {
    // Single line comment
    print("hello");
    
    /*
     * Multi-line comment
     */
    return 0;
}
```

### Parse Tests (`tests/parse/`)
Test the full parser and AST construction.

**Example Tests:**
- Function declaration parsing
- Variable declaration parsing
- Control flow parsing (if, while, for)
- Expression parsing (precedence, associativity)
- Structure and enum parsing
- Import/export parsing
- Error recovery

**Example:**
```rauma
// test_functions.rm
fn simple() {
    return 42;
}

fn with_params(a, b) {
    return a + b;
}

fn nested() {
    fn inner() {
        return "inner";
    }
    return inner();
}
```

### Type Tests (`tests/type/`)
Test the type checker and inference engine.

**Example Tests:**
- Type inference for variables
- Function type checking
- Structure type checking
- Generic type checking (future)
- Trait implementation checking (future)
- Type conversion validation
- Type error detection

**Example:**
```rauma
// test_inference.rm
fn test_inference() {
    let x = 42;          // inferred: int
    let y = 3.14;        // inferred: float
    let z = x + 5;       // x is int, 5 is int, z is int
    let w = y * 2.0;     // y is float, 2.0 is float, w is float
}
```

### Error Tests (`tests/error/`)
Test error handling and diagnostic reporting.

**Example Tests:**
- Syntax error detection
- Type error messages
- Undefined variable errors
- Duplicate declaration errors
- Import/export errors
- Error recovery quality
- Error message clarity

**Example:**
```rauma
// test_type_error.rm
// Should produce type error: cannot add int and str
fn type_error() {
    let x = 42;
    let y = "hello";
    let z = x + y;  // TYPE ERROR
}
```

### Interference Tests (`tests/interference/`)
Test the interference ownership system.

**Example Tests:**
- Read interference detection
- Write interference detection
- Move semantics validation
- Borrow checking
- Safe concurrent access
- Unsafe pattern detection

**Example:**
```rauma
// test_move.rm
struct Data {
    value: int;
}

fn consume(d Data) {
    // takes ownership
}

fn test_move() {
    let d = Data { value: 42 };
    consume(d);
    // ERROR: use after move
    print(d.value);
}
```

### Codegen Tests (`tests/codegen/`)
Test code generation and backend correctness.

**Example Tests:**
- C code generation
- LLVM IR generation (future)
- Machine code generation (future)
- Optimization correctness
- Debug information generation
- Linkage and symbol resolution

**Example:**
```rauma
// test_codegen.rm
// Should generate correct C code
fn add(a, b) {
    return a + b;
}

fn main() {
    let result = add(10, 20);
    print(result);
}
```

### Selfhost Tests (`tests/selfhost/`)
Test self-hosting compilation verification.

**Example Tests:**
- Bootstrap compiler equivalence
- Stage-to-stage consistency
- Fixed-point verification
- Performance regression detection
- Test suite consistency across stages

**Example:**
```rauma
// test_selfhost.rm
// Compiled by rmb, rmc1, rmc2, rmc3
// All should produce identical output
fn selfhost_test() {
    let compiler_version = get_compiler_version();
    print("Compiled by: {compiler_version}");
}
```

## Running Tests

### Test Runner
```bash
# Run all tests
rmb test

# Run specific test category
rmb test --category syntax
rmb test --category type

# Run single test file
rmb test tests/syntax/comments.rm

# Run with verbose output
rmb test --verbose

# Run and generate coverage report
rmb test --coverage
```

### Test Environment
```bash
# Setup test environment
rmb test --setup

# Clean test artifacts
rmb test --clean

# Update golden files (expected output)
rmb test --update
```

## Test Format

### Test Metadata
Each test file can include metadata comments:
```rauma
// Test: test_name
// Category: syntax
// Description: Test comment parsing
// Expected: Parses successfully
// Tags: comments, parsing
```

### Expected Output
Tests can specify expected output:
```rauma
// EXPECT stdout: "Hello, world!\n"
// EXPECT stderr: ""
// EXPECT exit: 0

fn main() {
    print("Hello, world!\n");
}
```

### Error Expectations
```rauma
// EXPECT error: "type mismatch"
// EXPECT line: 10
// EXPECT column: 15

fn type_error() {
    let x = 42 + "hello";  // Should produce type error
}
```

## Test Infrastructure

### Test Runner Features
- **Parallel Execution**: Run tests in parallel
- **Isolation**: Each test runs in separate process
- **Timeout**: Prevent hanging tests
- **Resource Limits**: Control memory and CPU usage
- **Snapshot Testing**: Compare output with expected snapshots
- **Property Testing**: Generate random test cases

### Test Utilities
```rauma
// test_utils.rm - Common test utilities
import std.test;

test_suite "compiler tests" {
    test "basic parsing" {
        let ast = parse("fn main() {}");
        assert_not_null(ast);
    }
    
    test "type checking" {
        let program = "let x = 42;";
        let typed = type_check(program);
        assert(typed.has_type("x", "int"));
    }
}
```

## Continuous Integration

### Automated Testing
- Run on every commit
- Multiple platforms (Linux, macOS, Windows)
- Multiple compilers (rmb, rmc stages)
- Performance regression detection
- Memory leak detection

### Test Coverage
- Line coverage reporting
- Branch coverage analysis
- Function coverage tracking
- Critical path coverage

### Quality Gates
- All tests must pass
- No performance regressions
- No new compiler warnings
- Coverage thresholds met
- Memory safety verified

## Writing Tests

### Guidelines
1. **One Concept Per Test**: Test a single feature or bug
2. **Descriptive Names**: Clearly indicate what's being tested
3. **Minimal Examples**: Use smallest possible test case
4. **Expected Behavior**: Specify what should happen
5. **Cleanup**: Tests should not leave artifacts

### Example Test Creation
```bash
# Create new test
rmb test --new tests/syntax/new_feature.rm

# Write test
echo '// Test: new_syntax
// Tests new language feature
fn test() {
    // New syntax here
}' > tests/syntax/new_feature.rm

# Run test
rmb test tests/syntax/new_feature.rm
```

## Test Maintenance

### Adding Tests
1. Place in appropriate category directory
2. Add descriptive metadata
3. Specify expected behavior
4. Verify test passes
5. Add to test suite if needed

### Updating Tests
1. Update when compiler behavior changes
2. Maintain backward compatibility for stable features
3. Document reason for change
4. Verify updated test passes

### Removing Tests
1. Only remove for removed features
2. Document in changelog
3. Update test counts

## Future Test Enhancements

### Planned Features
- **Fuzz Testing**: Random program generation
- **Differential Testing**: Compare with other compilers
- **Formal Verification**: Prove compiler correctness
- **Performance Testing**: Benchmark suite
- **Security Testing**: Vulnerability detection

### Integration Testing
- **Third-party Code**: Test with existing codebases
- **Compiler Plugins**: Test extension points
- **Tool Integration**: Test with IDEs, build systems
- **Cross-compilation**: Test multiple targets