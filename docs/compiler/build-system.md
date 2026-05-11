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

v0.0.9h keeps the bridge build system local and non-package-based while trying
the real `rmc/main.rm` graph. v0.0.9h-fix2 removes the first boolean/logical
cgen blocker: `rmb/tests/rmc_bool_bridge/main.rm` builds through `rmc0`, runs,
and emits C with `>=`, `<=`, `&&`, and `||`. The real graph still reaches Tier 0
only: `rmb` builds `rmc0`, but `rmc0 build ../rmc/main.rm` still reports
`build failed: unsupported source`. The local multi-file bridge and controlled
candidate chain are still the verified build paths.

v0.0.9h-fix3 pushes the same real graph through five additional blockers. The
bridge now parses/checks real `rmc/main.rm`, but the build still stops before
Tier 1 because generated C fails to compile. The next build-system blockers are
generated-C graph issues rather than package lookup: cross-module forward
references, duplicate compatibility macros, and remaining string-aware local
typing gaps.

v0.0.8r adds `rmc build <path>`, which wraps the full emit‑c→write‑file→cc‑compile pipeline behind a single command. The command writes generated C to `build/rmc_build_out.c`, compiles it to `build/rmc_build_out`, and reports success. This is still a bridge, not `rmc`’s own backend / link step. Later milestones replace the bridge with `rmc`’s own build system.

v0.0.8s expands that bridge subset while keeping it single-file only. Current
`rmc build` support is limited to:

- `fn main()` and `fn main(args Args)`
- `Args` command dispatch with `args_len`, `args_get`, and `str_eq`
- direct string literal prints from `main`
- local int variables initialized from int literals or simple calls
- one local string command variable from `args_get`
- printing local int variables
- simple int functions with zero, one, or two int parameters
- void helper functions and function call statements
- return literals, return identifiers, and binary `+` returns
- `return;` in void helpers / command dispatch
- assignment with `=`
- compound assignment with `+=`
- simple integer comparisons
- `if` / `else`
- `while`

Current limitations remain explicit: no `break`/`continue`, no structs, no
general string variables or string concatenation, no imports, no multi-file rmc
build, no chunk layout in `rmc`, and no HIR/MIR or full backend.

v0.0.8u adds a tiny proto self-build target:

```bash
./build/rmb build ../rmc/main.rm
./build/debug/native/bin/main build ../examples/selfbuild/tiny.rm
./build/rmc_build_out
```

This still uses the same single-file bridge output path
`build/rmc_build_out.c` / `build/rmc_build_out`. It proves that the
RauMa-written `rmc` can build a tiny RauMa program, but it is not multi-file
chunk building and not self-host fixed point.

v0.0.8v adds a larger proto self-build CLI tool target:

```bash
./build/rmb build ../rmc/main.rm
./build/debug/native/bin/main build ../examples/selfbuild/tool.rm
./build/rmc_build_out version
./build/rmc_build_out score
```

The generated tool uses `Args` command dispatch, helper functions, loops, and
conditionals, but it still flows through the same single-file bridge output
path. There is still no `rmc` chunk layout, multi-file build graph, package
manager, HIR/MIR, or fixed-point self-hosting.

v0.0.8w consolidates the bridge implementation without expanding the language
surface. The emitted C prelude and function wrappers are split into smaller
helpers, and regression fixtures cover the supported Args, math, control-flow,
and compound-assignment shapes. The build output path remains
`build/rmc_build_out.c` / `build/rmc_build_out`.

v0.0.8x adds `rmb/tests/rmc_readiness_probe.rm`, which exercises the current
bridge subset near its useful limit: Args dispatch, a helper function, a loop,
nested `if` checks, `str_eq`, and int output. The self-build readiness audit in
`docs/compiler/self-build-readiness.md` identifies multi-file `use` resolution,
chunk layout, structured AST storage, broader codegen, and real type checking
as the main build-system blockers before `rmc` can build meaningful parts of
itself.

v0.0.8y adds `examples/selfbuild/rmc-mini.rm` and
`rmb/tests/rmc_selfbuild_mini.rm`. This target is a single-file compiler-like
CLI built through the same bridge path. It verifies the current build bridge can
produce a standalone command binary with help/version/demo/check-style commands,
but the output path is still `build/rmc_build_out.c` / `build/rmc_build_out`
and there is still no multi-file `rmc` build graph.

v0.0.8z stabilizes this single-file bridge as the completed v0.0.8 behavior.
The stable bridge can build tiny/tool/probe/rmc-mini targets, but real chunk
builds in `rmc` remain future work. The verification checklist is captured in
`docs/compiler/v008-stabilization.md`, and the next fixed-point planning
boundary is `docs/compiler/v009-plan.md`.

v0.0.9a keeps the same bridge architecture and hardens `rmc-mini` with a
`self-test` command plus `rmb/tests/rmc_mini_hardening_probe.rm`. This is still
single-file bridge verification, not a chunk build graph.

v0.0.9b starts the `rmc` local multi-file foundation. `rmc build <entry.rm>`
can now discover top-level local `use` statements, resolve modules relative to
the entry file directory, emit all discovered modules into one bridge C file,
and compile/link that file through the existing `cc_compile` bridge.

v0.0.9c extends that foundation with the `rmb/tests/rmc_group_probe/` fixture,
which mirrors real `rmc` module shapes. Nested paths such as `source.span` and
`lex.token` resolve to `source/span.rm` and `lex/token.rm` beside the entry
file, and qualified calls such as `lex.token.print_token_name(...)` are emitted
with module-mangled C symbols. The bridge also handles the small transitive
dependencies used by the probe, for example `lex/lexer.rm` importing
`lex.token` and `source.source`.

v0.0.9d grows that probe into a broader real-ish CLI module group at
`rmb/tests/rmc_cli_probe/`. The fixture has eight modules under `cli/`,
`source/`, and `diag/` that exercise:

- `Args` parameters in dependency module functions (not only in `main`)
- `path str` parameters and string literal arguments wrapped through `rm_str`
- `read_file` / `str_len` / `str_byte` builtins inside dependency module
  functions (the bridge prelude now ships `rm_str_len`, `rm_str_byte`, and
  `rm_read_file`)
- qualified calls into nested namespaces such as `cli.file.print_file_info`,
  `source.span.print_span_size`, and `diag.output.error`
- dispatch helpers in the entry that combine multiple qualified calls

v0.0.9e adds real-ish frontend module group probes shaped like the real
compiler frontend at:

- `rmb/tests/rmc_frontend_lexer/` — lexer-shaped graph (`source.span`,
  `source.source`, `diag.output`, `lex.char`, `lex.token`, `lex.lexer`)
- `rmb/tests/rmc_frontend_parser/` — parser-shaped graph (`source.source`,
  `lex.token`, `parse.ast`, `parse.parser`, `diag.output`)
- `rmb/tests/rmc_frontend_checker/` — checker-shaped graph (`type.types`,
  `type.checker`, `diag.output`)
- `rmb/tests/rmc_frontend_combined/` — combined frontend graph that uses all
  four namespaces together (`source.*`, `lex.*`, `parse.*`, `type.*`,
  `diag.output`) under one entry

These probes exercise nested module paths with repeated module-name components
across the graph, transitive dependencies, void module helpers, qualified
calls into nested namespaces, and `int`-returning helpers consumed by control
flow. The combined group glues lexer/parser/checker modules into one
buildable graph without a one-file bundled compiler hack.

Current `rmc` multi-file limitations:

- local modules only
- small dependency graph (one-level transitive `use` walk, up to 16 direct
  modules at the entry; transitive deps deduped per graph)
- one generated C file, not per-module chunks
- no package lookup
- no stdlib lookup
- no chunk cache or incremental rebuild
- limited duplicate/cycle handling beyond rejecting unsupported/missing modules
- no real module interface or cross-module type checking yet

v0.0.9g-fix uses this same local multi-file bridge for the controlled
`rmb/tests/rmc_candidate/` fixed-point candidate chain. The real `rmc`
(`rmc0`) builds the candidate (`rmc1`), `rmc1` builds `rmc2`, and `rmc2` builds
`rmc3` for the exact candidate entry path. The candidate build command compiles
the current generated C artifact through the bridge output path, so this proves
the controlled candidate chain but not a general real-source compiler build.
- function names must be unique across modules in the same graph because
  per-module `#define rm_fn_<name>` macros would otherwise collide
- the parser/checker path is bypassed for multi-module builds; the bridge
  cgen scans tokens directly

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

## Current rmc Bridge Build Status

As of v0.0.9h-fix4, the bridge build path can compile the real multi-module
`rmc/main.rm` graph through Tier 4:

```text
rmc0 build ../rmc/main.rm
rmc1-real build ../rmc/main.rm
rmc2-real build ../rmc/main.rm
rmc3-real version/help
```

The build-system bridge still resolves modules relative to the entry file and
does not implement package management or stdlib lookup. The fix4 work only
hardens generated-C graph emission for the current real compiler graph:
function prototypes, scoped compatibility macros, nested dependency emission,
unused static helper handling, and string-aware `RmStr` lowering.

v0.0.9i verifies that this bridge path is deterministic for the real graph.
Generated C from `rmc0`, `rmc1-real`, and `rmc2-real` matches exactly with
SHA-256 `34e3bfe394347d852aa34db5dc753f6f22e63ed1a119d6d54d57b079da93db27`
and size `207067`. Behavior comparison across `rmc1-real`, `rmc2-real`, and
`rmc3-real` also passes.

## v0.1.0 Release Build

The v0.1.0 GitHub Actions release workflow uses the same bridge build path:

1. build `rmb`
2. build `rmc0`
3. verify `rmc0 -> rmc1-real -> rmc2-real -> rmc3-real`
4. compare generated C artifacts
5. compare behavior across real stages
6. build `examples/setup/rauma-setup.rm`
7. package `rauma-rmc`, `rauma-rmb`, `rauma-setup`, and checksums

The release workflow does not commit generated binaries. It publishes assets to
GitHub Releases using `GITHUB_TOKEN` and `gh release create`.

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
