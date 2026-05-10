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

### v0.0.9d build larger real rmc module groups

- grow from the probe fixture toward larger real `rmc` module groups
- keep package lookup, stdlib lookup, and fixed point out of scope
- use failures to identify the next smallest bridge and module graph gaps

### v0.0.9e rmc chunk/multi-file design hardening

- design `rmc`-side chunk build
- harden output paths, graph traversal, cycle diagnostics, and deterministic artifacts

### v0.0.9f fixed-point candidate

- `rmb` builds `rmc1`
- `rmc1` builds candidate compiler source
- compare outputs only when pipeline is stable

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
