# RauMa v0.0.9 Deterministic Self-Host Verification

## Chain

v0.0.9i rebuilds the real self-host chain with actual `rmc/main.rm` as the
target:

```text
rmb -> rmc0
rmc0 -> rmc1-real
rmc1-real -> rmc2-real
rmc2-real -> rmc3-real
```

The verified commands were:

```text
./build/rmb build ../rmc/main.rm
./build/debug/native/bin/main build ../rmc/main.rm
./build/rmc1-real.exe build ../rmc/main.rm
./build/rmc2-real.exe build ../rmc/main.rm
```

Each real build reported:

```text
build ok: build/rmc_build_out
```

`rmc1-real`, `rmc2-real`, and `rmc3-real` all reported:

```text
rmc 0.0.9h-fix
```

## Generated C comparison

The generated C artifacts were captured under `rmb/build/`:

```text
real_from_rmc0.c 34e3bfe394347d852aa34db5dc753f6f22e63ed1a119d6d54d57b079da93db27 207067
real_from_rmc1.c 34e3bfe394347d852aa34db5dc753f6f22e63ed1a119d6d54d57b079da93db27 207067
real_from_rmc2.c 34e3bfe394347d852aa34db5dc753f6f22e63ed1a119d6d54d57b079da93db27 207067
generated_c_compare= PASS
```

The comparison was exact byte-for-byte equality. No deterministic source fix
was needed for this milestone.

## Behavior comparison

The following commands were compared across `rmc1-real`, `rmc2-real`, and
`rmc3-real`:

```text
no args
version
help
demo-lex
demo-parse
lex tests/rmc_lex_input.rm
parse tests/rmc_parse_input.rm
check tests/rmc_check_ok.rm
emit-c tests/rmc_emit_workflow_add.rm
build tests/rmc_candidate/main.rm
wat
```

The outputs matched:

```text
behavior_compare= PASS
candidate_self_test_tail= candidate ok
```

## Regression coverage

The controlled candidate chain still reaches `rmc3-candidate`; all candidate
versions print `rmc-candidate 0.0.9f`, and all self-tests end with
`candidate ok`.

The previous frontend combined fixture, CLI probe, group probe, rmc-mini,
boolean bridge fixture, old rmc command smokes, `project_basic`,
`build_hello`, and `make test` all passed.

## Remaining limitations

This is deterministic bridge self-host verification, not a full compiler
architecture milestone. RauMa still does not have package management, stdlib
lookup, HIR/MIR, LLVM/rmgen/rmlink, release polish, or a general module graph
algorithm beyond the current local bridge needs.

## v0.1.0 release carry-forward

v0.1.0 carries this deterministic chain into release automation. GitHub Actions
CI runs the verification script, and the release workflow packages the final
verified compiler as `rauma-rmc`.

The release also includes:

- `rauma-rmb` for bootstrap recovery
- `rauma-setup`, built from `examples/setup/rauma-setup.rm`
- `SHA256SUMS.txt` for release asset verification

The generated C comparison and behavior comparison remain the release gate for
the current self-hosted bridge compiler.
