# RauMa

RauMa is a compiled, no-runtime, server-side-oriented programming language.

## What's New in v0.2.0

- **Full struct and enum support** - `struct`, `enum`, and `pub` keyword now fully parsed and codegen'd
- **Type system foundation** - `rmc/type/types.rm` and `rmc/type/symtab.rm` define primitive types, symbol table entries, and type resolution
- **HIR/MIR infrastructure** - Initial `rmc/hir/` and `rmc/mir/` modules define type/value nodes and optimization primitives
- **Standard library modules** - `std/core/`, `std/str/`, `std/io/`, `std/math/` with core utilities
- **LSP language server** - `rmc/lsp/` module with JSON-RPC protocol support, keyword/builtin completions
- **Enhanced CI/CD** - Cross-platform tests for stdlib, HIR/MIR, and LSP modules
- **Project manager** - `rauma/main.rm` provides the package/build/test driver

## Overview

- Source files use the `.rm` extension
- C-like syntax with strong type inference
- Optional explicit types
- Error syntax using `!!`
- Future ownership/safety model based on Interference Ownership / Interference Type
- Compiler is designed to self-host

## Current Status

RauMa has two compiler tracks:

- `rmc` is the self-hosted RauMa compiler written in RauMa.
- `rmb` is the C11 bootstrap compiler kept for recovery.
- `rauma` is the project manager written in RauMa; it delegates compilation to `rmc`.
- `rauma-setup.sh` and `rauma-setup.ps1` are the v0.2.0 installer scripts.

As of v0.2.0, the product compiler path is the C backend pipeline:
source -> lexer -> parser -> AST -> checker -> C codegen -> native binary.
The release gate builds `rmc/main.rm`, checks the stdlib/tooling modules, runs
the LSP smoke path, and builds/runs the stdlib smoke program.

## Repository Structure

- `rmb/` - RauMa Bootstrap Compiler (C11)
  - Small, stable bootstrap compiler
  - Compiles RauMa bootstrap subset into C
  - Foundation for building the main compiler

- `rmc/` - RauMa Main Compiler (RauMa `.rm`)
  - Real compiler written in RauMa
  - Architecture: Lexer -> Parser -> Type Checker -> HIR -> MIR -> C Backend
  - Includes `rmc/hir/` (HIR types/nodes), `rmc/mir/` (MIR types/optimization), `rmc/lsp/` (language server)
  - Includes `rmc/type/` (type system, symbol table)

- `std/` - RauMa Standard Library
  - `std/core/` - Option types, core utilities
  - `std/str/` - String operations
  - `std/io/` - Input/output
  - `std/math/` - Mathematical functions
  - `std/conv/` - Type conversions (planned)

- `tests/` - Test Suites
  - Syntax, parsing, type checking, error handling, codegen, self-hosting tests

- `docs/` - Documentation
  - Language specification, compiler architecture, roadmap

- `tools/` - Build and Development Tools

- `rauma/` - RauMa Project Manager (RauMa `.rm`)

## Project Manager

`rauma` is the project-level command, while `rmc` remains the compiler backend.

```bash
rauma new hello
cd hello
rauma check
rauma build
rauma run
rauma test
```

Current project-manager features:

- `rauma.toml` creation and validation
- local module graph output in `.rauma/module-graph.json` with existence status
- source diagnostics with module line numbers before compiler delegation
- `build`, `run`, `test`, `fmt`, `graph`, `doctor`, and local dependency `add`
- test discovery for `tests/basic.rm` and `tests/test1.rm` through `tests/test16.rm`
- checksum-based incremental build cache in `.rauma/build-cache.txt`

## Build Model

- Chunk/file based compilation
- Each `.rm` source file compiles into its own build chunk
- Changing a file rebuilds only affected chunk and dependent chunks when public interface changes

## Backends

1. C backend (bridge) - for bootstrap stability
2. LLVM backend - delayed until after full self-hosting
3. Other backends (rmgen, rmlink) possible later

## Design Principles

1. RauMa must eventually self-host
2. `rmb` must stay small, boring, stable, and C11-only
3. `rmb` emits C first, not LLVM
4. `rmc` uses Lexer -> Parser -> Type Checker -> HIR -> MIR -> Backend architecture
5. No macros, async, HTTP, package registry, LLVM, VM, or native linker in bootstrap stage
6. Public API type-explicit, local code inference-friendly
7. Simple C-family syntax

## Getting Started

### Download

Download the archive for your platform from GitHub Releases:

- `rauma-v0.2.0-windows-x64.zip`
- `rauma-v0.2.0-windows-arm64.zip`
- `rauma-v0.2.0-linux-x64.tar.gz`
- `rauma-v0.2.0-linux-arm64.tar.gz`
- `rauma-v0.2.0-macos-arm64.tar.gz`

Each archive includes:

- `rmc-<platform>` - self-hosted RauMa compiler
- `rauma-<platform>` - RauMa project manager
- `rmb-<platform>` - C11 bootstrap compiler for recovery
- `rauma-setup.sh` / `rauma-setup.ps1` - script installers

Put the matching `rmc` and `rauma` binaries on `PATH`.

```bash
rmc version
rauma version
```

Linux/macOS can use `rauma-setup.sh`; Windows can use `rauma-setup.ps1`.
Both scripts support dry-run mode and install the compiler as `rmc`.

### Build from source

```bash
# Build the bootstrap compiler
cd rmb
make

# Build the RauMa-written compiler
./build/rmb build ../rmc/main.rm
./build/debug/native/bin/main version

# Verify the product compiler pipeline
powershell -NoProfile -ExecutionPolicy Bypass -File ../scripts/release/verify-product.ps1

# Dry-run the script installer
powershell -NoProfile -ExecutionPolicy Bypass -File ../scripts/install/rauma-setup.ps1 -DryRun -Version v0.2.0
```

Release packages are produced by GitHub Actions from tag `v0.2.0`; generated
binaries are not committed to the repository.

## License

See [LICENSE](LICENSE) file.
