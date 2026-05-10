# RauMa v0.0.9 Fixed-Point Plan

## Purpose

v0.0.9 is about approaching self-host fixed point.

It must not claim fixed point is done until the compiler pipeline can prove the
expected staged build chain and comparison strategy.

## Proposed phases

### v0.0.9a Standalone rmc-mini hardening

- keep single-file
- expand `rmc-mini` only if needed
- add stricter regression tests
- v0.0.9a adds the `self-test` command to `rmc-mini`
- verified before any fixed-point or real `rmc` self-build attempt

### v0.0.9b rmc multi-file build foundation

- add local `use`/module resolution to `rmc build`
- build small local multi-file programs without a one-file bundled compiler hack
- emit one bridge C file for the discovered local module graph
- keep package lookup, stdlib lookup, chunk caching, and fixed point for later

### v0.0.9c real rmc module group build

- use the multi-file foundation to build a small real-ish `rmc` module group
- exercise nested local modules such as `cli.help`, `source.span`, and
  `lex.token`
- verify transitive local dependencies and qualified calls without a one-file
  bundled compiler hack
- keep scope below full `rmc` self-hosting
- identify remaining source-shape/codegen blockers

### v0.0.9d larger rmc CLI/source/diag module group build

- grew the multi-file probe to a real-ish `rmc-cli`-shaped graph at
  `rmb/tests/rmc_cli_probe/` with eight modules across `cli/`, `source/`, and
  `diag/`
- exercised `Args` and `path str` parameters in dependency module functions,
  qualified calls into nested namespaces, diagnostic-style void helpers, and
  `read_file`/`str_len`/`str_byte` builtins inside module functions
- added the `rm_str_len`, `rm_str_byte`, and `rm_read_file` runtime helpers to
  the bridge prelude so dependency-module file input works
- kept package lookup, stdlib lookup, full type checking, and the fixed point
  out of scope
- this validates a broader real-ish module topology, not full real `rmc`

### v0.0.9e lexer-focused rmc module group build

- grow the local module-group fixture toward a lexer-shaped graph
- exercise larger token/byte/scan helpers without rewriting the real lexer
- still no fixed point, no full real `rmc` build

### v0.0.9f parser/checker module group build

- grow the local module-group fixture toward parser/checker-shaped graphs
- still no fixed point, no full real `rmc` build

### v0.0.9g fixed-point candidate planning

- design `rmc`-side chunk build, output paths, graph traversal, cycle
  diagnostics, and deterministic artifacts
- once the pipeline is stable: `rmb` builds `rmc1`, `rmc1` builds candidate
  compiler source, compare outputs

## Do not do yet

- do not jump directly to full self-host
- do not rewrite `rmc` backend
- do not add package manager
- do not add LLVM/rmgen/rmlink
- do not chase full language completeness before the fixed-point path is clear

## Required prerequisites

- multi-file support or one-file bundling strategy
- more general codegen
- better parser/codegen model
- stable output paths
- deterministic build artifacts
- clear comparison strategy
