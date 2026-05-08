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

### Components (v0.0.6)
1. **Source Loading**: Read `.rm` files
2. **Lexer/Tokenizer**: Convert source to tokens with span tracking
3. **Diagnostics**: Error reporting with source locations
4. **Parser**: Recursive descent parser building AST with improved diagnostics
5. **AST**: Abstract Syntax Tree representation with spans and match/enum payload support
6. **Type Checker**: Basic type checking and inference
7. **C Code Generator**: Emits portable C99/C11 from checked AST and invokes `gcc`

### Build Pipeline (v0.0.6)
```
.rm source
    ↓
Source Loader → RmbSource
    ↓
Lexer → tokens (with spans)
    ↓
Parser → AST (Abstract Syntax Tree)
    ↓
Type Checker → typed AST view
    ↓
C Code Generator → build/<name>.c
    ↓
gcc → build/<name> executable
```

## Language Support (v0.0.6)

### Lexer Features
- **Keywords**: `fn`, `pub`, `struct`, `enum`, `use`, `return`, `if`, `else`, `while`, `for`, `match`, `case`, `const`, `defer`, `true`, `false`, `none`
- **Identifiers**: ASCII letters, digits, underscores
- **Integers**: Decimal literals
- **Strings**: Double-quoted with escape sequences (`\n`, `\r`, `\t`, `\0`, `\"`, `\\`)
- **Comments**: Line comments (`//`) and block comments (`/* */`)
- **Operators/Punctuation**: All basic RauMa operators

### Parser Features (v0.0.5)
- **Top-level items**: `use`, `fn`, `struct`, `enum` declarations
- **Function declarations**: Parameters, return types, error types (`!!`), body blocks
- **Struct declarations**: Fields with types (including qualified, pointer, slice, array, optional types)
- **Enum declarations**: Simple variants and variants with named payload fields (`Variant(name Type)`)
- **Statements**: Variable declarations (`:=`, `: Type =`), assignments, control flow (`if`, `while`, `for`), `return`, `defer`, `match` with `case` patterns
- **Match statements**: Parse match expression with cases, optional binding patterns (`case Variant(binding)`)
- **Expressions**: Identifiers, literals, calls, field access, unary/binary operators, error operators (`?`, `!`), `else` expressions
- **Type references**: Simple types, pointers (`*T`), slices (`[]T`), arrays (`[N]T`), optional (`T?`), qualified (`module.Type`)
- **Improved diagnostics**: Clear error messages with source spans and recovery

### Type Checker Features (v0.0.5)
- **Primitive types**: `int`, `uint`, `float`, `byte`, `bool`, `str`, `void`
- **Type forms**: named, pointer (`*T`), slice (`[]T`), array (`[N]T`), optional (`T?`)
- **Symbol collection**: functions, structs, enums collected before bodies
- **Local inference**: `name := expr` infers type from expression
- **Explicit declaration**: `name: Type = expr` checks assignability
- **Public function rule**: `pub fn` parameters must have explicit types
- **Returns**: checked against declared return type or inferred
- **Error syntax**: `!!`, `?`, `!`, and `else` validated against function context
- **Optional handling**: `none` only assigns to optional types

### C Codegen Features (v0.0.6)
- **Targets portable C99/C11**: standard headers only (`stdint.h`, `stddef.h`, `stdbool.h`, `stdio.h`, `string.h`, `stdlib.h`)
- **Type mapping**: `int`→`int64_t`, `uint`→`uint64_t`, `float`→`double`, `byte`→`uint8_t`, `bool`→`bool`, `str`→`RmStr`
- **Structs**: `struct User { ... }` → `typedef struct Rm_User { ... } Rm_User;`
- **Functions**: prefixed `rm_fn_<name>`; `main` becomes the C entry point
- **Built-in `print`**: emits `rm_print(rm_str(...))`, `rm_print_int`, or `rm_print_bool` depending on argument type
- **Statements**: variable declarations (`:=`, `: T = e`), assignment (`=`, `+=`, `-=`, `*=`, `/=`), `return`, `if`/`else`, `while`, `for`
- **Expressions**: int/string/bool literals, identifiers, calls, field access, unary (`-`, `!`, `&`, `*`), binary arithmetic and comparisons, `&&`/`||`

### Build Limitations (v0.0.6)
- No error-returning function codegen (`fn ... !! E`, `?`, `!`)
- No `match` codegen
- No `defer` codegen
- No module / chunk build yet (single-file builds only)
- No optional/`else` codegen
- No struct literal codegen
- No HIR / MIR / optimizer / native backend

### Unsupported Features (in v0.0.6)
- HIR / MIR
- Full module resolution across files (qualified types treated as opaque names)
- Generics, traits, overloads
- Implicit numeric conversions
- Ownership / interference checking

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

## Usage (v0.0.6)

```bash
# Show help
./build/rmb help

# Tokenize a RauMa file
./build/rmb lex source.rm

# Parse a RauMa file and show AST summary
./build/rmb parse source.rm

# Type-check a RauMa file
./build/rmb check source.rm

# Build a RauMa file to executable (writes build/<name>.c and build/<name>)
./build/rmb build source.rm

# Show version
./build/rmb version
```

### Build output (temporary v0.0.6 layout)
```
tests/foo.rm  →  build/foo.c  →  build/foo
```
The full chunk build system is planned for v0.0.7.

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