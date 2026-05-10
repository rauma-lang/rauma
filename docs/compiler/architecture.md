# Compiler Architecture

## Overview

RauMa uses a multi-stage compiler architecture designed for incremental development and self-hosting.

### Bootstrap pipeline status (v0.0.8z)

The full pipeline (Resolver → HIR → MIR → multi-backend) is the long-term design.
The v0.0.7 bootstrap compiler `rmb` deliberately skips most of these stages and
still emits C directly from checked AST chunks:

```
.rm source files → Build Driver → Lexer → Parser → AST → Type Checker → C Code Generator → gcc → executable
```

- **The build driver coordinates multiple file chunks.** It resolves simple
  local `use` declarations, emits one `.c` and `.o` per chunk, then links all
  objects into one executable.
- **HIR is still planned but not implemented in `rmb`.**
- **MIR is still planned but not implemented in `rmb`.**
- **The C backend in `rmb` is a bootstrap backend** — small, boring, and
  table-driven from the AST. It will eventually be replaced (or sit alongside)
  the richer pipeline once `rmc` takes over.
- **v0.0.8o adds an `rmc emit-c <path>` bridge.** It reuses parser/checker
  helpers and emits C text for tiny recognized source shapes. This is not the
  full backend and `rmc` still does not compile or link file contents.
- **v0.0.8p verifies the external compile workflow that consumes
  `rmc emit-c` output:**

  ```
  .rm source → rmc emit-c → C text on stdout
            ↘ user/Makefile redirects to a .c file
                                     ↓
                              external gcc
                                     ↓
                                  executable
  ```

  `rmc` has no build, link, or process-execution stage of its own yet — the
  external pipeline above is intentionally the only way to turn rmc-emitted
  C into a binary at this milestone.

  v0.0.8q adds two temporary bridge builtins to `rmb`'s runtime (`write_file`,
  `cc_compile`). These allow compiled RauMa programs to write files and invoke
  `gcc` via `system(3)`. They are **bridge‑only** and exist to enable `rmc`’s
  own `rmc build` command.

  v0.0.8r implements `rmc build <path>`, which reads a file, checks it, emits C,
  writes to `build/rmc_build_out.c`, compiles to `build/rmc_build_out`, and reports
  success. This is still a bridge using the external C compiler, not the full
  backend.

  v0.0.8s expands that bridge emitter beyond two fixed templates. The
  RauMa-written `rmc` can now emit/build several simple single-file programs
  involving int locals, int returns, simple function calls, and int prints.
  This is still direct bridge output from token stream scans, not HIR/MIR or a
  full backend.

  v0.0.8t adds a minimal control-flow bridge subset: assignment, `+=`, integer
  comparisons, `if`/`else`, and `while`. The emitter is still single-file and
  scanner-driven, not a full backend.

  v0.0.8u adds a proto self-build chain: `rmb` builds `rmc`, then the generated
  `rmc` builds `examples/selfbuild/tiny.rm`, and the produced executable runs.
  This is a tiny target proof, not self-host fixed point.

  v0.0.8v extends that proto chain with `examples/selfbuild/tool.rm`, a
  single-file CLI tool using `Args`, command dispatch, helper functions, loops,
  and conditionals. `rmc` can build the tool and the produced executable handles
  multiple commands, but this remains bridge output rather than HIR/MIR or a
  full backend.

  v0.0.8w consolidates the bridge emitter by splitting common C prelude and
  wrapper emission into smaller RauMa helpers and adding regression fixtures for
  the supported single-file subset. This is cleanup before self-build readiness
  work, not a full backend.

  v0.0.8x audits the `rmc/` source tree against the current bridge subset and
  adds a readiness probe. The audit lives at
  `docs/compiler/self-build-readiness.md` and identifies the remaining gaps
  before `rmc` can build meaningful parts of itself.

  v0.0.8y adds `examples/selfbuild/rmc-mini.rm`, a standalone compiler-like CLI
  target with no imports. `rmb` builds `rmc`, `rmc` builds `rmc-mini`, and the
  produced binary handles version, lexer demo, parser demo, and check demo
  commands. This is still not fixed-point self-hosting.

  v0.0.8z stabilizes the bridge architecture and documents the completed
  v0.0.8 verification chain in `docs/compiler/v008-stabilization.md`. The
  v0.0.9 boundary is planned in `docs/compiler/v009-plan.md`. There is still no
  HIR/MIR-backed `rmc` backend and no fixed-point self-hosting.

  v0.0.9a hardens `rmc-mini` with a self-test command that exercises helper
  calls, loops, conditionals, and int output. This keeps the bridge target
  standalone and does not introduce HIR/MIR, a full backend, or fixed-point
  self-hosting.

  v0.0.9b adds a local module graph foundation to `rmc build`. The bridge can
  resolve simple local `use` modules relative to the entry file and emit one
  combined C file for small multi-file programs. This preserves the bridge
  architecture and still does not add HIR/MIR, package resolution, chunk
  caching, or fixed-point self-hosting.

  v0.0.9g verifies the first candidate generation in the staged chain:
  `rmb -> rmc0 -> rmc1 candidate`. The candidate behavior is stable for its
  supported demo commands, but the architecture is not a fixed point yet because
  the candidate lacks a real `build <path>` command and cannot produce `rmc2`.

  v0.0.9c builds a real-ish `rmc` module group probe using nested local modules
  (`cli.help`, `source.span`, `lex.token`, and friends). The bridge now handles
  the small transitive graph and qualified calls needed by that probe, while
  still emitting one combined C file and deferring real chunking, HIR/MIR, and
  fixed point.

  v0.0.9d adds a larger CLI/source/diag module probe at
  `rmb/tests/rmc_cli_probe/` with eight modules. The bridge cgen now handles
  `Args` and `path str` parameters in dependency module functions,
  `read_file`/`str_len`/`str_byte` builtins inside module functions, and
  string literal arguments wrapped through `rm_str` for qualified calls. Empty
  `return;` in module void functions emits valid C (`return;` instead of
  `return 0;`). This continues to grow the verified topology without HIR/MIR,
  per-module chunk caching, or fixed-point self-hosting.

  v0.0.9e adds real-ish frontend module group probes at
  `rmb/tests/rmc_frontend_lexer/`, `rmb/tests/rmc_frontend_parser/`,
  `rmb/tests/rmc_frontend_checker/`, and `rmb/tests/rmc_frontend_combined/`.
  Each probe is shaped like a slice of the real compiler frontend with nested
  module paths (`source.span`, `lex.token`, `parse.parser`, `type.checker`),
  transitive local dependencies, repeated module-name components across the
  graph, void module helpers, and qualified calls into nested namespaces. The
  combined group glues lexer/parser/checker modules into one buildable graph.
  This is still bridge cgen, not real chunking, HIR/MIR, or fixed point.

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
build/debug/native/c/chunk/src/main/main.c
    ↓
build/debug/native/c/chunk/src/main/main.o
    ↓
build/debug/native/bin/main
```

### Dependency Tracking
- v0.0.7 has file-level dependency discovery from simple local `use`
  declarations.
- `.rmi` interface summaries are emitted for every chunk.
- Interface hashing, affected-chunk rebuilds, and parallel compilation remain
  future work.

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
- In v0.0.8z, `rmc` has a minimal CLI skeleton plus early data modules:
  `source/span`, `source/source`, `lex/token`, `lex/lexer`, `ast/ast`,
  `lex/stream`, `parse/parser`, `type/checker`, `cgen/cgen`, `diag/output`,
  `cli/args`, and `cli/file`.
- The lexer scans hardcoded source bytes, and the parser consumes cursor-style
  stream helpers. The parser handles a small file-driven subset and computes
  summaries for multiple functions, parameters, optional return types, return,
  variable, and call statement shapes. Token text helpers print names from
  source spans, but AST storage is still minimal and summary-oriented. There
  are no token arrays, full type inference, name resolution, HIR/MIR, full
  backend architecture, or self-hosting logic yet. The current `build` command
  is a single-file bridge around emit-C, file write, and external C compilation.
  It now handles a tiny imperative subset and can build the tiny self-build
  example, but still has no `break`/`continue`, structs, multi-file
  compilation, HIR/MIR, fixed-point self-hosting, or full backend. It can build
  the tiny self-build target and a small self-build CLI tool target, but it
  still cannot build itself. The bridge emitter has consolidated prelude and
  wrapper helpers, but it remains direct token-stream C emission rather than a
  HIR/MIR-backed backend. The readiness audit identifies the next likely target
  as a standalone `rmc-mini.rm`, not the real multi-file `rmc` tree. v0.0.8y
  verifies that target, but the real `rmc` tree still requires module/chunk
  support before it can be built by `rmc`. v0.0.8z closes this as the bridge
  milestone and defers the next self-host path to v0.0.9 planning.

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
