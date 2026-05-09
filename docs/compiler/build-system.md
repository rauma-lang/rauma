# Build System

## Overview

RauMa uses a chunk-based build system designed for incremental compilation, parallel builds, and efficient dependency management.

## v0.0.7 bootstrap chunk layout

The v0.0.7 bootstrap compiler `rmb` implements a small, explicit chunk build
layout for local projects. Each `.rm` source file becomes one generated C file,
one object file, and one textual interface file:

```
build/debug/native/c/chunk/<source-dir>/<file-stem>/<file-stem>.c
build/debug/native/c/chunk/<source-dir>/<file-stem>/<file-stem>.o
build/debug/native/c/chunk/<source-dir>/<file-stem>/<file-stem>.rmi
build/debug/native/bin/<entry-stem>
```

`rmb build <file>`:
1. lex / parse / type-check the input
2. recursively resolve local `use` declarations from the entry file directory
3. topologically order chunks with dependencies before dependents
4. emit one portable C11 file and one `.rmi` per chunk
5. compile each generated C file to an object with `gcc`
6. link all objects into `build/debug/native/bin/<entry-stem>`

For v0.0.7, `profile = debug` and `target = native` are fixed. There are no
CLI flags for profile or target yet.

### Local `use` resolution

`use` is local and single-package only:

```rauma
use math;
use app.util;
```

Given entry file `tests/project_nested/main.rm`, `use app.util;` resolves to:

```
tests/project_nested/app/util.rm
```

Rules:
- resolve relative to the entry file directory
- map `.` to `/`
- append `.rm`
- do not search global package paths
- do not resolve `std` or external modules yet

### Current non-incremental behavior

v0.0.7 always rebuilds the discovered chunks for each `rmb build <file>` run.
The `.rmi` file is a deterministic text summary of public top-level
declarations, but it is not used for cache invalidation yet.

Future incremental work should hash source files and `.rmi` interfaces, skip
unchanged implementation chunks, and rebuild dependents only when imported
interfaces change.

## v0.0.8p external rmc compile workflow (temporary)

`rmc` does not yet have its own build/link stage. While the RauMa-written
compiler is being grown, the milestone-level pipeline is **temporary** and
goes through an external C compiler:

```bash
rmc emit-c input.rm > out.c
gcc -std=c11 -Wall -Wextra -Werror -pedantic out.c -o out
./out
```

`rmc` only produces C text on stdout. The user (or a Makefile target such as
`rmb/`'s `test-rmc-emit-workflow`) is responsible for capturing that output to
a file, invoking `gcc`, and running the binary. `rmc` itself does **not**:

- invoke `gcc` or any other tool
- spawn processes
- write files

v0.0.8q adds two temporary builtins to `rmb` that allow RauMa programs to
write files and invoke the external C compiler (`write_file`, `cc_compile`).
These bridge primitives are used by the `rmb/tests/build_write_file.rm` and
`rmb/tests/build_cc_compile.rm` fixtures to verify the low‑level operations
work.

A future `rmc build` command (planned for v0.0.8r) will wrap the full
emit‑c→write‑file→cc‑compile pipeline behind a single command, and later
milestones replace the bridge with `rmc`’s own backend / link step.

## Chunk-Based Architecture

### What is a Chunk?
A chunk is the compilation unit in RauMa. Each `.rm` source file becomes a chunk during compilation.

### Chunk Structure
```
src/main.rm
    ↓
build/<profile>/<target>/c/chunk/src/main/
    ├── main.c          # Generated C code
    ├── main.o          # Compiled object
    ├── main.a          # Static library
    ├── interface.rmh   # Public interface hash
    └── deps.json       # Dependency information
```

### Why Chunks?
- **Incremental**: Only rebuild changed chunks
- **Parallel**: Compile multiple chunks simultaneously  
- **Caching**: Reuse unchanged compilation results
- **Distribution**: Distribute compilation across machines
- **Modular**: Clear boundaries between components

## Build Pipeline

### 1. Source Analysis
```
.rm source file
    ↓
Parse → AST
    ↓
Extract public interface
    ↓
Compute interface hash
```

### 2. Dependency Resolution
```
interface hash
    ↓
Compare with cached hash
    ↓
If changed: mark chunk for rebuild
    ↓
Mark dependent chunks for rebuild
```

### 3. Compilation
```
Chunk marked for rebuild
    ↓
Generate C code (or other backend)
    ↓
Compile to object file
    ↓
Update cache with new hash
```

### 4. Linking
```
All object files ready
    ↓
Link into executable/library
    ↓
Optional: strip, optimize, sign
```

## Directory Structure

### Source Layout
```
src/
├── main.rm           # Entry point
├── utils/
│   ├── math.rm
│   └── strings.rm
└── api/
    ├── client.rm
    └── server.rm
```

### Build Output
```
build/
├── debug/
│   ├── x86_64-linux/
│   │   ├── c/
│   │   │   ├── chunk/
│   │   │   │   ├── src/
│   │   │   │   │   ├── main/
│   │   │   │   │   ├── utils/
│   │   │   │   │   └── api/
│   │   │   │   └── deps/
│   │   │   ├── obj/
│   │   │   └── bin/
│   │   └── llvm/     # Future
│   └── aarch64-macos/
├── release/
└── test/
```

## Dependency Tracking

### Interface Hashing
Each chunk's public interface is hashed:
- Function signatures (name, parameters, return type)
- Structure definitions
- Constant values
- Type aliases
- Import/export declarations

### Dependency Graph
Build system maintains a graph:
- Nodes: chunks
- Edges: "depends on" relationships
- When interface changes, dependent chunks rebuild

### Smart Rebuild Detection
1. **Source unchanged**: Skip compilation
2. **Interface unchanged**: Skip dependent rebuilds  
3. **Implementation changed**: Recompile chunk only
4. **Interface changed**: Recompile chunk + dependents

## Build Profiles

### Debug Profile
```
Features:
- No optimizations (-O0)
- Full debug symbols
- Assertions enabled
- Sanitizers optional
- Fast compilation

Use: Development, debugging
```

### Release Profile
```
Features:
- Maximum optimizations (-O3)
- Stripped debug symbols
- Assertions disabled
- LTO (Link Time Optimization)
- Size/performance tuned

Use: Production deployment
```

### Size Profile
```
Features:
- Optimize for size (-Os)
- Minimal debug info
- Aggressive dead code elimination
- Function/data section reordering

Use: Embedded, constrained environments
```

### Test Profile
```
Features:
- Instrumentation for coverage
- Debug symbols
- Test harness linking
- Memory checking tools

Use: Automated testing
```

## Build Commands

### Basic Commands
```bash
# Build project
rmb build

# Build specific target
rmb build --target x86_64-linux

# Build with profile
rmb build --profile release

# Clean build artifacts
rmb clean

# Run tests
rmb test
```

### Advanced Commands
```bash
# Generate compilation database
rmb compile-commands

# Show dependency graph
rmb deps

# Build single file
rmb build src/main.rm

# Parallel build (N jobs)
rmb build -j8

# Verbose output
rmb build -v
```

## Cache Management

### Cache Locations
```
~/.cache/rauma/
    ├── build/
    │   ├── <project-hash>/
    │   │   ├── chunks/
    │   │   ├── objects/
    │   │   └── metadata/
    └── compiler/
        └── <version>/
```

### Cache Invalidation
Cache invalidated when:
- Compiler version changes
- Build flags change  
- Source files change (detected via hash)
- System libraries change

### Cache Pruning
- Automatic cleanup of old builds
- Configurable retention policy
- Manual cache clearing command

## Cross-Compilation

### Target Specification
```bash
rmb build --target x86_64-linux-gnu
rmb build --target aarch64-linux-android
rmb build --target wasm32-wasi
```

### Toolchain Configuration
```
~/.config/rauma/toolchains/
    ├── x86_64-linux-gnu.toml
    ├── aarch64-linux-android.toml
    └── wasm32-wasi.toml
```

### Toolchain File Format
```toml
[target]
arch = "x86_64"
os = "linux"
abi = "gnu"

[tools]
cc = "x86_64-linux-gnu-gcc"
ar = "x86_64-linux-gnu-ar"
ld = "x86_64-linux-gnu-ld"

[flags]
cflags = ["-march=x86-64", "-mtune=generic"]
ldflags = ["-static-libgcc"]
```

## Integration with IDEs

### Compilation Database
Generate `compile_commands.json` for:
- clangd LSP server
- YouCompleteMe
- ccls
- Other C/C++ tools

### Language Server Protocol
- Real-time diagnostics
- Code completion
- Go-to definition
- Find references
- Refactoring support

### Build System Hooks
- Pre-build scripts
- Post-build scripts
- Custom compilation steps
- Asset processing

## Future Extensions

### Distributed Builds
- Share compilation across network
- Build farm support
- Cloud build services

### Binary Caching
- Share compiled artifacts between developers
- CI/CD artifact reuse
- Versioned binary packages

### Live Reload
- Watch mode for development
- Hot code replacement
- Instant feedback loop

### Plugin System
- Custom build steps
- Alternative backends
- Code generators
- Analysis tools
