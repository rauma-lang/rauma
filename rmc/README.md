# RauMa Main Compiler (rmc)

## Overview

`rmc` is the main RauMa compiler, written in RauMa itself. It implements the full RauMa language and is designed to eventually compile itself (self-hosting).

## Architecture

### Compilation Pipeline
```
.rm source files
    ↓
Lexer → tokens
    ↓
Parser → AST
    ↓
Resolver → resolved AST
    ↓
Type Checker → typed AST
    ↓
HIR Generator → HIR
    ↓
MIR Generator → MIR
    ↓
Backend → object files
    ↓
Linker → executable
```

### Module Structure
```
rmc/
├── cli/          # Command-line interface
├── driver/       # Compilation driver
├── source/       # Source file management
├── lex/          # Lexer/tokenizer
├── parse/        # Parser
├── ast/          # Abstract Syntax Tree
├── resolve/      # Name resolution
├── type/         # Type checking
├── interfere/    # Interference checking
├── hir/          # High-Level IR
├── mir/          # Middle-Level IR
├── cgen/         # C code generation
├── diag/         # Diagnostics
└── build/        # Build system
```

## Self-Hosting Process

### Bootstrap Stages
1. **Stage 0**: `rmb` (C bootstrap compiler) compiles `rmc1`
2. **Stage 1**: `rmc1` compiles `rmc2` 
3. **Stage 2**: `rmc2` compiles `rmc3`
4. **Verification**: Ensure `rmc2` and `rmc3` produce identical output

### Bootstrap Subset
The initial `rmc` (compiled by `rmb`) implements only a subset of RauMa:
- Basic types and control flow
- Functions and structures
- Simple error handling
- Minimal standard library

Later versions (compiled by previous `rmc` versions) implement:
- Full type system
- Advanced error handling
- Standard library
- Optimization passes
- Multiple backends

## Building

### Initial Build (using rmb)
```bash
# From rmb directory
cd ../rmb
make

# Build first rmc
./build/rmb build ../rmc
```

### Self-Hosting Build
```bash
# After rmc can compile itself
cd rmc
rmc build .
```

### Development Build
```bash
# Build with debug symbols
rmc build --profile debug

# Build with optimizations
rmc build --profile release

# Build and run tests
rmc test
```

## Language Features

### Full RauMa Support
- **Type System**: Inference, generics, traits, sum types
- **Error Handling**: `!!`, `?`, `else err`, error types
- **Memory Safety**: Interference-based ownership
- **Concurrency**: Built-in safe concurrency primitives
- **Metaprogramming**: Compile-time evaluation, macros (future)

### Backend Support
- **C Backend**: Portable C99/C11 output
- **LLVM Backend**: Native code generation (future)
- **Custom Backends**: WebAssembly, specialized targets (future)

### Optimization
- **High-Level**: Inlining, constant folding, dead code elimination
- **Mid-Level**: Common subexpression elimination, loop optimizations
- **Low-Level**: Instruction selection, register allocation

## Development

### Code Style
- Follow RauMa language conventions
- Use type inference where appropriate
- Explicit types at public API boundaries
- Comprehensive error handling
- Documentation for public APIs

### Testing
- Unit tests for each module
- Integration tests for compilation pipeline
- Property tests for language features
- Self-hosting verification tests

### Documentation
- API documentation in source comments
- Architecture documentation in `docs/`
- Examples in `examples/`
- Test documentation in test files

## Relationship with rmb

### rmb (Bootstrap)
- Written in C11
- Minimal, stable
- Only C backend
- Builds first rmc

### rmc (Main)
- Written in RauMa
- Full-featured
- Multiple backends
- Self-hosting
- Active development

### Migration Path
1. New features added to `rmc`
2. `rmb` remains stable
3. `rmc` evolves independently
4. Eventually `rmc` replaces `rmb` for all development

## Future Directions

### Language Evolution
- Additional type system features
- Better concurrency support
- Enhanced metaprogramming
- Improved error messages

### Compiler Improvements
- Incremental compilation
- Parallel compilation
- Better optimization passes
- Enhanced debugging support

### Tooling Integration
- Language server protocol
- IDE plugins
- Debugger integration
- Profiling tools

### Ecosystem
- Package manager
- Build system improvements
- Cross-compilation support
- Community libraries