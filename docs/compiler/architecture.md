# Compiler Architecture

## Overview

RauMa uses a multi-stage compiler architecture designed for incremental development and self-hosting.

### Bootstrap pipeline status (v0.0.6)

The full pipeline (Resolver → HIR → MIR → multi-backend) is the long-term design.
The v0.0.6 bootstrap compiler `rmb` deliberately skips most of these stages and
emits C directly from the checked AST:

```
.rm source → Lexer → Parser → AST → Type Checker → C Code Generator → gcc → executable
```

- **HIR is still planned but not implemented in `rmb`.**
- **MIR is still planned but not implemented in `rmb`.**
- **The C backend in `rmb` is a bootstrap backend** — small, boring, and
  table-driven from the AST. It will eventually be replaced (or sit alongside)
  the richer pipeline once `rmc` takes over.

## Compilation Pipeline

```
.rm source files
    ↓
Tokenizer/Lexer
    ↓ tokens
    ↓
Parser
    ↓ AST (Abstract Syntax Tree)
    ↓
Resolver
    ↓ resolved AST (with symbols)
    ↓
Type Checker
    ↓ typed AST (with types)
    ↓
HIR Generator
    ↓ HIR (High-Level IR)
    ↓
MIR Generator
    ↓ MIR (Middle-Level IR)
    ↓
Backend (C, LLVM, etc.)
    ↓ object files
    ↓
Linker
    ↓ executable/library
```

## Component Details

### 1. Tokenizer/Lexer
- Converts source text to token stream
- Handles whitespace, comments, literals, identifiers, operators
- Tracks source locations (line, column, file)
- Produces `Token` stream with spans

### 2. Parser
- Recursive descent parser
- Builds AST from token stream
- Handles operator precedence, associativity
- Basic error recovery
- Produces `AstNode` tree

### 3. Resolver
- Binds identifiers to declarations
- Handles scopes, namespaces, imports
- Detects undefined variables
- Creates symbol tables
- Produces `ResolvedAst` with symbol references

### 4. Type Checker
- Infers types for expressions
- Checks type compatibility
- Validates function signatures
- Handles generic types (future)
- Produces `TypedAst` with type annotations

### 5. HIR Generator
- Lowers AST to High-Level IR
- Simplifies complex expressions
- Normalizes control flow
- Prepares for optimization
- Produces `HirNode` graph

### 6. MIR Generator
- Lowers HIR to Middle-Level IR
- Platform-independent representation
- Explicit memory operations
- Basic block structure
- SSA form (Static Single Assignment)
- Produces `MirFunction` with basic blocks

### 7. Backend
- Target-specific code generation
- Multiple backend options:
  - **C Backend**: Generates C99/C11 code
  - **LLVM Backend**: Generates LLVM IR (future)
  - **Custom Backend**: Direct machine code (future)
- Register allocation, instruction selection
- Produces object files or assembly

### 8. Linker
- Combines object files
- Resolves external references
- Creates final executable or library
- Handles platform-specific binary format

## Intermediate Representations

### AST (Abstract Syntax Tree)
- Close to source syntax
- Preserves all source information
- Used for early analysis and diagnostics
- Language-specific

### HIR (High-Level IR)
- Simplified AST
- Normalized expressions
- Explicit control flow
- Still language-oriented
- Platform-independent

### MIR (Middle-Level IR)
- Lower-level representation
- Explicit memory operations
- Basic block structure
- SSA form for optimization
- Backend input format

## Build System Integration

### Chunk-Based Compilation
```
src/main.rm
    ↓
build/<profile>/<target>/c/chunk/src/main/main.c
    ↓
build/<profile>/<target>/c/chunk/src/main/main.o
    ↓
build/<profile>/<target>/c/chunk/src/main/main.a
```

### Dependency Tracking
- File-level dependencies
- Interface hash for incremental builds
- Rebuild only affected chunks
- Parallel compilation support

### Profile Support
- **Debug**: Full debugging info, no optimizations
- **Release**: Optimized, no debugging info
- **Size**: Optimized for smallest binary
- **Test**: Instrumented for testing

## Bootstrap vs Main Compiler

### rmb (Bootstrap)
- Written in C11
- Minimal feature set
- Only C backend
- Stable, boring codebase
- Builds first rmc

### rmc (Main Compiler)
- Written in RauMa
- Full feature set
- Multiple backends
- Self-hosting
- Active development

## Self-Hosting Process

```
rmb (C) builds → rmc1 (RauMa)
rmc1 builds → rmc2 (RauMa)
rmc2 builds → rmc3 (RauMa)

Verify: rmc2 == rmc3 (behavior and tests)
```

## Error Reporting

### Diagnostic System
- Rich error messages with source context
- Multiple error levels: error, warning, info, hint
- Source location tracking
- Suggested fixes
- Error recovery where possible

### Error Categories
- **Syntax Errors**: Invalid source code
- **Semantic Errors**: Type mismatches, undefined symbols
- **Logic Errors**: Dead code, unreachable paths
- **Safety Errors**: Interference violations, use after free

## Optimization Pipeline

### High-Level Optimizations
- Constant folding
- Dead code elimination
- Function inlining
- Loop optimizations

### Mid-Level Optimizations
- Common subexpression elimination
- Copy propagation
- Strength reduction
- Register allocation

### Low-Level Optimizations
- Instruction scheduling
- Peephole optimizations
- Vectorization (future)
- Parallelization (future)

## Testing Architecture

### Unit Tests
- Individual compiler components
- Tokenizer, parser, type checker tests
- Isolated, fast tests

### Integration Tests
- End-to-end compilation tests
- Full program compilation
- Multiple backend tests

### Regression Tests
- Historical bug fixes
- Performance benchmarks
- Compatibility tests

## Future Extensions

### Additional Backends
- WebAssembly backend
- GPU/accelerator backends
- Custom hardware backends

### Language Features
- Macros/metaprogramming
- Reflection/introspection
- Foreign function interface

### Tooling Integration
- Language server protocol
- IDE plugins
- Debugger integration
- Profiling tools