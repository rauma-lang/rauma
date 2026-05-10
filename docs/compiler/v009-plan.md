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

### v0.0.9b rmc source partition audit

- choose which real `rmc` modules can be represented as standalone or bundled source
- identify imports/use blockers

### v0.0.9c single-file bundled rmc experiment

- generate or write a bundled `rmc-onefile.rm`
- no multi-file build yet
- prove `rmc build rmc-onefile.rm` if feasible

### v0.0.9d multi-file/chunk plan

- design `rmc`-side chunk build
- do not implement until design is clear

### v0.0.9e fixed-point candidate

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
