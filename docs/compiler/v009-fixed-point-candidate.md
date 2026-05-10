# RauMa v0.0.9 Fixed-Point Candidate

## What was verified

The first v0.0.9g attempt verified only the first candidate generation:

- `rmb -> rmc0`
- `rmc0 -> rmc1 candidate`

`rmc0` is the real RauMa-written `rmc` built by `rmb`.
`rmc1` is the controlled compiler-shaped candidate built from
`rmb/tests/rmc_candidate/main.rm`.

That attempt stopped because the candidate fixture had only `build-demo`, not a
real `build <path>` command.

The v0.0.9g-fix milestone adds a controlled candidate `build <path>` command
and verifies this chain:

- `rmb -> rmc0`
- `rmc0 -> rmc1 candidate`
- `rmc1 -> rmc2 candidate`
- `rmc2 -> rmc3 candidate`

`rmc1`, `rmc2`, and `rmc3` all ran these commands with matching behavior:

- no args
- `version`
- `lex-demo`
- `parse-demo`
- `check-demo`
- `emit-demo`
- `build-demo`
- `self-test`
- unknown command

The candidate self-test output ends with:

```text
candidate ok
```

The controlled candidate build command supports only:

```text
tests/rmc_candidate/main.rm
```

For that path it prints:

```text
candidate build ok: build/rmc_build_out
```

## What this proves

- The bootstrap compiler `rmb` can still build the real RauMa-written `rmc`.
- The real RauMa-written `rmc` can build the controlled compiler-shaped
  candidate.
- A candidate-built compiler can build the controlled candidate again.
- Candidate behavior is stable across `rmc1`, `rmc2`, and `rmc3` for the
  supported commands.
- The current bridge can compile a deeper compiler-shaped multi-file fixture.

## What this does not prove

- The real `rmc` is not self-hosting yet.
- The controlled candidate fixed-point chain is not the real compiler
  fixed-point chain.
- The candidate `build <path>` command supports only the exact controlled
  candidate path.
- The real multi-file `rmc/main.rm` is not built by `rmc` yet.
- There is no full package lookup, stdlib lookup, HIR/MIR, LLVM backend, or
  fixed-point comparison pipeline.

## Artifact comparison

Generated C comparison was performed for the controlled chain. The C produced
by `rmc1 build tests/rmc_candidate/main.rm` matched the C produced by
`rmc2 build tests/rmc_candidate/main.rm` exactly.

Behavior comparison across `rmc1`, `rmc2`, and `rmc3` also matched for all
candidate commands listed above.

## Remaining gaps to v0.1.0

- v0.0.9h attempted the real `rmc/main.rm` graph and reached Tier 0 only:
  `rmb` still builds `rmc0`, but `rmc0 build ../rmc/main.rm` remains blocked by
  unsupported boolean/logical expression lowering in real lexer code.
- Preserve candidate build outputs under distinct paths for staged comparison.
- Expand real `rmc` source coverage beyond controlled fixtures.
- Add robust cross-module checking and diagnostics.
- Define deterministic artifact comparison rules.
- Harden regression coverage for the staged build chain.
- Keep docs and command-line build UX aligned with the actual verified chain.
