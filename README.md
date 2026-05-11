# RauMa

RauMa is a compiled, no-runtime, server-side-oriented programming language.

## Overview

- Source files use the `.rm` extension
- C-like syntax with strong type inference
- Optional explicit types
- Error syntax using `!!`
- Future ownership/safety model based on Interference Ownership / Interference Type
- Compiler is designed to self-host

## Current Status

RauMa has two compiler tracks:

- `rauma-rmc` / `rmc` is the self-hosted RauMa compiler written in RauMa.
- `rauma-rmb` / `rmb` is the C11 bootstrap compiler kept for recovery.
- `rauma-setup` is a RauMa-written setup helper.

As of v0.1.0, the real self-host chain is verified:

```text
rmb -> rmc0 -> rmc1-real -> rmc2-real -> rmc3-real
```

Generated C exact comparison passes across the real stages, behavior comparison
passes across `rmc1-real`, `rmc2-real`, and `rmc3-real`, and `make test`
passes. The compiler remains a bridge C-backend release with a local module
graph only: no package manager, stdlib lookup, HIR/MIR, LLVM, rmgen, or rmlink.

See `docs/compiler/v009-deterministic-self-host.md` and
`docs/compiler/v009-plan.md` for the verified chain and remaining boundaries.

## Repository Structure

- `rmb/` - RauMa Bootstrap Compiler (C11)
  - Small, stable bootstrap compiler
  - Compiles RauMa bootstrap subset into C
  - Foundation for building the main compiler

- `rmc/` - RauMa Main Compiler (RauMa `.rm`)
  - Real compiler written in RauMa
  - Will eventually compile itself
  - Architecture: AST -> HIR -> MIR -> backend

- `std/` - RauMa Standard Library
  - Minimal standard library
  - Core functionality only

- `tests/` - Test Suites
  - Syntax, parsing, type checking, error handling, interference, codegen, self-hosting tests

- `examples/` - Example Programs
  - Simple `.rm` examples

- `docs/` - Documentation
  - Language specification, compiler architecture, roadmap

- `tools/` - Build and Development Tools

## Build Model

- Chunk/file based compilation
- Each `.rm` source file compiles into its own build chunk
- Changing a file rebuilds only affected chunk and dependent chunks when public interface changes

## Backends

1. C backend first (for bootstrap stability)
2. LLVM backend delayed until after self-hosting
3. Other backends (rmgen, rmlink) possible later

## Design Principles

1. RauMa must eventually self-host
2. `rmb` must stay small, boring, stable, and C11-only
3. `rmb` emits C first, not LLVM
4. `rmc` uses AST -> HIR -> MIR -> backend architecture
5. No macros, async, HTTP, package registry, LLVM, VM, or native linker in bootstrap stage
6. Public API type-explicit, local code inference-friendly
7. Simple C-family syntax

## Getting Started

### Download

Download `rauma-v0.1.0-windows-x64.zip` from GitHub Releases, unpack it, and
put `rauma-rmc-windows-x64.exe` on `PATH`.

```bash
rauma-rmc-windows-x64.exe version
rauma-setup-windows-x64.exe doctor
```

### Build from source

```bash
# Build the bootstrap compiler
cd rmb
make

# Build the RauMa-written compiler
./build/rmb build ../rmc/main.rm
./build/debug/native/bin/main version

# Verify the real self-host chain
powershell -NoProfile -ExecutionPolicy Bypass -File ../scripts/release/verify-self-host.ps1

# Build the RauMa-written setup helper
./build/debug/native/bin/main build ../examples/setup/rauma-setup.rm
./build/rmc_build_out doctor
```

Release packages are produced by GitHub Actions from tag `v0.1.0`; generated
binaries are not committed to the repository.

## License

See [LICENSE](LICENSE) file.
