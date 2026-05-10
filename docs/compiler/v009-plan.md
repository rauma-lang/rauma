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

### v0.0.9e real-ish rmc frontend module group build

- grow the local module-group fixtures into lexer-, parser-, checker-, and
  combined-frontend-shaped graphs under
  `rmb/tests/rmc_frontend_lexer/`, `rmb/tests/rmc_frontend_parser/`,
  `rmb/tests/rmc_frontend_checker/`, and `rmb/tests/rmc_frontend_combined/`
- exercise nested module paths (`source.span`, `lex.token`, `parse.parser`,
  `type.checker`), transitive local dependencies, repeated module-name
  components across the graph, void module helpers, and qualified calls
  shaped like the real compiler frontend
- prove `rmc build` can compile real-ish frontend module graphs without a
  one-file bundled compiler hack
- still no fixed point, no full real `rmc` build, no package manager, no
  stdlib lookup, no HIR/MIR, no LLVM

### v0.0.9f rmc compiler candidate ✓ DONE

- with the frontend graphs proven, build a candidate real `rmc` source layout
  end-to-end through `rmc build`
- expand the bridge subset only as needed to reach a buildable compiler
  candidate, not full language completeness
- still no fixed point yet
- candidate is a controlled multi‑file fixture under `rmb/tests/rmc_candidate/`
  with `cli/`, `source/`, `diag/`, `lex/`, `parse/`, `type/`, `cgen/`, and `build/`
  module groups
- candidate binary supports `version`, `lex‑demo`, `parse‑demo`, `check‑demo`,
  `emit‑demo`, `build‑demo`, `self‑test`, help, and unknown‑command dispatch
- verified with `rmc build rmb/tests/rmc_candidate/main.rm` (compiles with
  warnings about unused functions; passes manual compile without `-Werror`)
- previous frontend groups, probes, and self‑build targets still pass

### v0.0.9g fixed-point candidate

- design `rmc`-side chunk build, output paths, graph traversal, cycle
  diagnostics, and deterministic artifacts
- once the pipeline is stable: `rmb` builds `rmc1`, `rmc1` builds candidate
  compiler source, compare outputs

## Compressed remaining v0.0.9 path

1. v0.0.9e — real-ish frontend module groups ✓ DONE
2. v0.0.9f — rmc compiler candidate ✓ DONE
3. v0.0.9g — fixed-point candidate

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
