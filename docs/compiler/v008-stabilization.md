# RauMa v0.0.8 Stabilization

## Status

v0.0.8 is the `rmc` bridge milestone.

It proves:

- `rmb` builds the RauMa-written `rmc`
- `rmc` can lex real files
- `rmc` can parse real files for the supported subset
- `rmc` can run lightweight checks
- `rmc` can emit C for supported single-file programs
- emitted C can compile externally
- `rmc` can build supported single-file programs through the bridge
- `rmc` can build:
  - `examples/selfbuild/tiny.rm`
  - `examples/selfbuild/tool.rm`
  - `examples/selfbuild/rmc-mini.rm`

This is not self-hosting yet.

## Verified chain

The stabilized bridge chain is:

```text
rmb -> rmc -> rmc-mini -> working CLI binary
```

The supporting proto self-build chains are:

```text
rmb -> rmc -> tiny/tool/probe -> working binaries
```

## Supported rmc bridge subset

The v0.0.8 bridge supports:

- single-file source
- `fn`
- `fn main()`
- `fn main(args Args)`
- int parameters
- int returns
- void helper functions
- local int vars
- command string from `args_get`
- assignment
- compound assignment
- return expr
- `return;`
- direct function calls
- print string literal
- print int variable
- int literals
- string literals in direct print
- binary `+`
- integer comparisons
- `if/else`
- `while`
- `Args` builtins
- `str_eq`
- `read_file` / `write_file` / `cc_compile` bridge builtins

## Explicit non-goals

v0.0.8 explicitly does not include:

- no fixed point
- no `rmc` building real `rmc`
- no multi-file `rmc build`
- no chunk layout in `rmc`
- no imports/use in `rmc build`
- no HIR/MIR
- no full backend
- no package manager
- no standard library expansion

## Stable verification commands

Run from inside `rmb/`:

```bash
make clean
make
./build/rmb version
./build/rmb build ../rmc/main.rm
./build/debug/native/bin/main version

./build/debug/native/bin/main build tests/rmc_selfbuild_mini.rm
./build/rmc_build_out
./build/rmc_build_out version
./build/rmc_build_out lex-demo
./build/rmc_build_out parse-demo
./build/rmc_build_out check-demo
./build/rmc_build_out self-test

./build/debug/native/bin/main build tests/rmc_selfbuild_tool.rm
./build/rmc_build_out version
./build/rmc_build_out score

./build/debug/native/bin/main build tests/rmc_selfbuild_tiny.rm
./build/rmc_build_out

./build/debug/native/bin/main build tests/rmc_readiness_probe.rm
./build/rmc_build_out run

./build/debug/native/bin/main check tests/rmc_check_ok.rm
./build/debug/native/bin/main parse tests/rmc_parse_names.rm
./build/debug/native/bin/main lex tests/rmc_lex_input.rm
./build/debug/native/bin/main emit-c tests/rmc_emit_workflow_add.rm

./build/rmb build tests/project_basic/main.rm
./build/debug/native/bin/main

./build/rmb build tests/build_hello.rm
./build/debug/native/bin/build_hello

make test
```

If `make` is unavailable, build `rmb` directly with the documented C11 `gcc`
command and skip `make test`.

## Release checklist

- `git status --short` is clean before stabilization changes
- generated files are not tracked
- build artifacts stay ignored
- `make test` passes when `make` is available
- docs are updated
- roadmap is updated
- `git status --short` is clean after the stabilization commit
