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

- `rmb` is the C11 bootstrap compiler and remains the authoritative build path.
- `rmc` is the RauMa-written bridge compiler built by `rmb`.

As of v0.0.8z, `rmc build` supports a limited single-file bridge subset. It can
build the proto self-build targets under `examples/selfbuild/`, including
`tiny.rm`, `tool.rm`, and `rmc-mini.rm`. This proves the bridge chain, but it is
not self-hosting yet: `rmc` does not build the real multi-file `rmc` tree.

See `docs/compiler/v008-stabilization.md` for the stabilized bridge checklist
and `docs/compiler/v009-plan.md` for fixed-point planning boundaries.

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

```bash
# Build the bootstrap compiler
cd rmb
make

# Run the bootstrap compiler
./build/rmb --help
```

## License

See [LICENSE](LICENSE) file.
