# RauMa Bootstrap Compiler (rmb)

## Overview

`rmb` is the bootstrap compiler for RauMa, written in C11. It provides a minimal, stable foundation for building the main RauMa compiler.

## Purpose

- **Bootstrap**: Compile the first version of `rmc` (main RauMa compiler)
- **Stability**: Provide a reliable base that doesn't change often
- **Simplicity**: Implement only essential RauMa language features
- **Portability**: Generate C code that works on many platforms

## Design Principles

### 1. Stay Small
- Minimal feature set
- No unnecessary abstractions
- Focus on correctness over features

### 2. Stay Boring
- Stable C11 codebase
- Conservative changes only
- Well-tested core

### 3. Stay Fast
- Simple algorithms
- Direct code generation
- Minimal overhead

### 4. Stay Portable
- Standard C11 only
- No platform-specific code
- Generate portable C output

## Architecture

### Components (v0.0.3)
1. **Source Loading**: Read `.rm` files
2. **Lexer/Tokenizer**: Convert source to tokens with span tracking
3. **Diagnostics**: Error reporting with source locations
4. **Parser**: Recursive descent parser building AST
5. **AST**: Abstract Syntax Tree representation

### Build Pipeline (v0.0.3)
```
.rm source
    ↓
Source Loader → RmbSource
    ↓
Lexer → tokens (with spans)
    ↓
Parser → AST (Abstract Syntax Tree)
    ↓
AST Printer → human-readable summary
```

## Language Support (v0.0.3)

### Lexer Features
- **Keywords**: `fn`, `pub`, `struct`, `enum`, `use`, `return`, `if`, `else`, `while`, `for`, `match`, `case`, `const`, `defer`, `true`, `false`, `none`
- **Identifiers**: ASCII letters, digits, underscores
- **Integers**: Decimal literals
- **Strings**: Double-quoted with escape sequences (`\n`, `\r`, `\t`, `\0`, `\"`, `\\`)
- **Comments**: Line comments (`//`) and block comments (`/* */`)
- **Operators/Punctuation**: All basic RauMa operators

### Parser Features (v0.0.3)
- **Top-level items**: `use`, `fn`, `struct`, `enum` declarations
- **Function declarations**: Parameters, return types, error types (`!!`), body blocks
- **Struct declarations**: Fields with types
- **Enum declarations**: Simple and tuple variants
- **Statements**: Variable declarations (`:=`, `: Type =`), assignments, control flow (`if`, `while`, `for`), `return`, `defer`, `match`
- **Expressions**: Identifiers, literals, calls, field access, unary/binary operators, error operators (`?`, `!`), `else` expressions
- **Type references**: Simple types, pointers (`*T`), slices (`[]T`), arrays (`[N]T`), optional (`T?`), qualified (`module.Type`)

### Unsupported Features (in v0.0.3)
- Type checking
- Semantic analysis
- Code generation
- Module resolution (beyond parsing `use` statements)

## Building

### Prerequisites
- C11 compiler (gcc, clang, or equivalent)
- Make

### Build Commands
```bash
# Build compiler
make

# Clean build artifacts
make clean

# Run tests (when implemented)
make test

# Run the compiler
make run
```

### Output
The compiler executable is built at:
```
rmb/build/rmb
```

## Usage (v0.0.3)

```bash
# Show help
./build/rmb help

# Tokenize a RauMa file
./build/rmb lex source.rm

# Parse a RauMa file and show AST summary
./build/rmb parse source.rm

# Show version
./build/rmb version
```

## Development

### Code Style
- C11 with standard library only
- No external dependencies
- Consistent naming: `rmb_` prefix for public APIs
- Clear error handling
- Extensive comments for complex logic

### Testing Strategy
- Unit tests for each component
- Integration tests for full compilation
- Comparison tests with expected output
- Property tests for random valid programs

### Contribution Guidelines
1. Keep changes minimal and focused
2. Add tests for new functionality
3. Update documentation
4. Maintain backward compatibility
5. Follow existing code style

## Future Evolution

### Stability Guarantee
- `rmb` will remain stable after bootstrap completes
- Only critical bug fixes will be accepted
- No new features will be added

### Long-term Role
- Reference implementation for bootstrap subset
- Educational resource for compiler basics
- Fallback compiler if self-hosting fails

### Potential Extensions
- Cross-compilation support
- Better error messages
- Performance improvements (without complexity)

## Related Components

### rmc (Main Compiler)
- Written in RauMa
- Full language support
- Self-hosting
- Active development

### Tests
- Located in `rmb/tests/`
- Verify bootstrap compiler correctness
- Ensure stable behavior

### Documentation
- See `docs/compiler/bootstrap.md`
- Bootstrap process details
- Self-hosting verification