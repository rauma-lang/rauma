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

### Components (v0.0.8h)
1. **Source Loading**: Read `.rm` files
2. **Lexer/Tokenizer**: Convert source to tokens with span tracking
3. **Diagnostics**: Error reporting with source locations
4. **Parser**: Recursive descent parser building AST with improved diagnostics
5. **AST**: Abstract Syntax Tree representation with spans and match/enum payload support
6. **Type Checker**: Basic type checking and inference
7. **C Code Generator**: Emits portable C99/C11 from checked AST chunks
8. **Chunk Build Driver**: Resolves simple local `use` declarations, compiles per-file C chunks, and links objects with `gcc`

### Build Pipeline (v0.0.8h)
```
.rm entry source
    ↓
Build Driver → local use graph
    ↓
Source Loader / Lexer / Parser per chunk
    ↓
AST per chunk
    ↓
Type Checker per chunk
    ↓
C Code Generator → build/debug/native/c/chunk/<source-dir>/<stem>/<stem>.c
    ↓
gcc -c → per-chunk .o
    ↓
gcc link → build/debug/native/bin/<entry-stem>
```

## Language Support (v0.0.8h)

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
- **Opaque CLI type**: `Args` for `fn main(args Args)` only
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
- **String builtins**: `str_len(s str) int` and `str_byte(s str, index int) int` for bootstrap byte scanning
- **CLI/string/file helpers**: `args_len(args Args) int`, `args_get(args Args, index int) str`, `str_eq(a str, b str) bool`, and `read_file(path str) str`
- **Bridge builtins (v0.0.8q, temporary)**: `write_file(path str, text str) bool` and `cc_compile(c_path str, out_path str) int`
- **Statements**: variable declarations (`:=`, `: T = e`), assignment (`=`, `+=`, `-=`, `*=`, `/=`), `return`, `if`/`else`, `while`, `for`
- **Expressions**: int/string/bool literals, identifiers, calls, field access, unary (`-`, `!`, `&`, `*`), binary arithmetic and comparisons, `&&`/`||`

### Chunk Build Features (v0.0.7)
- **File chunks**: each `.rm` file emits its own `.c`, `.o`, and `.rmi`
- **Fixed layout**: `build/debug/native/...`
- **Simple local `use`**: `use math;` and `use app.util;`
- **Dependency walk**: recursively collects local dependencies from `use`
- **Cycle/missing file diagnostics**: fails build for simple use cycles and unresolved use targets
- **Linking**: links all chunk objects into one executable

### Build Limitations (v0.0.7)
- Local single-package `use` only
- No global package search
- No `std` modules
- No full incremental rebuild or cache invalidation
- No full module type-checking or public/private semantic enforcement
- No error-returning function codegen (`fn ... !! E`, `?`, `!`)
- No `match` codegen
- No `defer` codegen
- No optional/`else` codegen
- No struct literal codegen
- String builtins are byte-level only; no Unicode, no char type, no `s[i]` syntax
- `Args` is only a bootstrap CLI wrapper, not a general array/slice type
- `read_file` returns empty string on failure and leaks returned buffers for now
- No HIR / MIR / optimizer / native backend

### Bridge builtins (v0.0.8q)

`write_file(path str, text str) bool`
- writes all bytes of `text` to `path`, overwriting any existing file
- returns `true` on success, `false` on any failure (open/write/close)
- no append mode, no directory creation, no error message API
- path may not be null-terminated (it is copied to a temporary C string)

`cc_compile(c_path str, out_path str) int`
- temporary bridge that invokes `gcc -std=c11 -Wall -Wextra -Werror -pedantic
  <c_path> -o <out_path>` via `system(3)`
- returns the process exit code (0 on success)
- expects `gcc` on `PATH`; the host platform must have a working C compiler
- paths are concatenated into the command string with no shell escaping —
  **paths with spaces or shell metacharacters are not supported**
- there is no full process API and no general filesystem API yet; these two
  primitives only exist to unblock `rmc build` (planned for v0.0.8r)

### Unsupported Features (in v0.0.7)
- HIR / MIR
- Full module resolution across files
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

# Verify the rmc emit-c -> gcc -> run pipeline (also runs from `make test`)
make test-rmc-emit-workflow

# Run the compiler
make run
```

`test-rmc-emit-workflow` builds `rmb`, builds `rmc` via `rmb`, runs `rmc emit-c`
on the `rmc_emit_workflow_*.rm` fixtures, compiles the emitted C with `gcc`,
runs the resulting binaries, and checks their stdout matches the expected
output. The failure fixture is asserted to print `check failed` without
emitting any C.

### Proto self-build verification

v0.0.8u adds a tiny target buildable by the RauMa-written `rmc`:

```bash
./build/rmb build ../rmc/main.rm
./build/debug/native/bin/main build tests/rmc_selfbuild_tiny.rm
./build/rmc_build_out
```

Expected output:

```text
tiny self-build
ok
42
```

The user-facing copy lives at `examples/selfbuild/tiny.rm`. This verifies the
chain `rmb -> rmc -> tiny program`; it is not self-host fixed point.

v0.0.8v adds a larger CLI-tool target buildable by the RauMa-written `rmc`:

```bash
./build/rmb build ../rmc/main.rm
./build/debug/native/bin/main build tests/rmc_selfbuild_tool.rm
./build/rmc_build_out version
./build/rmc_build_out score
```

The user-facing copy lives at `examples/selfbuild/tool.rm`. It exercises
`Args`, command dispatch, helper functions, loops, and conditionals through the
same single-file bridge build path. This is still proto self-build, not `rmc`
building itself.

v0.0.8x adds a readiness probe and audit:

```bash
./build/debug/native/bin/main build tests/rmc_readiness_probe.rm
./build/rmc_build_out run
```

The expected probe output is `probe ok` followed by `15`. The audit lives at
`docs/compiler/self-build-readiness.md` and lists the remaining gaps before
`rmc` can build meaningful parts of itself.

v0.0.8y adds a standalone `rmc-mini` target:

```bash
./build/debug/native/bin/main build tests/rmc_selfbuild_mini.rm
./build/rmc_build_out version
./build/rmc_build_out check-demo
```

The user-facing copy lives at `examples/selfbuild/rmc-mini.rm`. It is a
single-file, import-free compiler-like CLI built by the RauMa-written `rmc`.
This is still not fixed-point self-hosting.

### Output
The compiler executable is built at:
```
rmb/build/rmb
```

## Usage (v0.0.8h)

```bash
# Show help
./build/rmb help

# Tokenize a RauMa file
./build/rmb lex source.rm

# Parse a RauMa file and show AST summary
./build/rmb parse source.rm

# Type-check a RauMa file
./build/rmb check source.rm

# Build a RauMa file or small local project to executable
./build/rmb build source.rm

# Show version
./build/rmb version
```

### Build output layout
```
tests/foo.rm
  -> build/debug/native/c/chunk/tests/foo/foo.c
  -> build/debug/native/c/chunk/tests/foo/foo.o
  -> build/debug/native/c/chunk/tests/foo/foo.rmi
  -> build/debug/native/bin/foo
```

For project builds:

```rauma
use app.util;
```

Given entry file `tests/project_nested/main.rm`, the build driver resolves that
use to `tests/project_nested/app/util.rm`. Dots map to path separators and
`.rm` is appended. The search is relative to the entry file directory only.

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
- v0.0.8a starts as a minimal RauMa-written skeleton under `rmc/`
- v0.0.8h adds `Args`-based command dispatch for help/version/demo commands
- v0.0.8i adds a temporary `read_file` smoke path for generated binaries
- Currently built by `rmb build ../rmc/main.rm`
- Full language support and self-hosting are later milestones

### Tests
- Located in `rmb/tests/`
- Verify bootstrap compiler correctness
- Ensure stable behavior

### Documentation
- See `docs/compiler/bootstrap.md`
- Bootstrap process details
- Self-hosting verification
