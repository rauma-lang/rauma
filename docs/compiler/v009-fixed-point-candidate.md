# RauMa v0.0.9 Fixed-Point Candidate

## What was verified

The v0.0.9g attempt verified the first candidate generation:

- `rmb -> rmc0`
- `rmc0 -> rmc1 candidate`

`rmc0` is the real RauMa-written `rmc` built by `rmb`.
`rmc1` is the controlled compiler-shaped candidate built from
`rmb/tests/rmc_candidate/main.rm`.

The produced `rmc1` candidate binary ran these commands:

- no args
- `version`
- `lex-demo`
- `parse-demo`
- `check-demo`
- `emit-demo`
- `build-demo`
- `self-test`
- unknown command

The candidate self-test ended with:

```text
candidate ok
```

The attempted `rmc1 -> rmc2` step did not pass. `rmc1` currently prints
`unknown command` for `build tests/rmc_candidate/main.rm` because the controlled
candidate fixture has only `build-demo`; it does not yet implement a real
`build <path>` command.

`rmc2` and `rmc3` were therefore not valid fixed-point generations in v0.0.9g.

## What this proves

- The bootstrap compiler `rmb` can still build the real RauMa-written `rmc`.
- The real RauMa-written `rmc` can build the controlled compiler-shaped
  candidate.
- The candidate binary preserves the expected compiler-like behavior for its
  supported demo commands.
- The current bridge can compile a deeper compiler-shaped multi-file fixture.

## What this does not prove

- The real `rmc` is not self-hosting yet.
- The controlled candidate is not a fixed point yet.
- `rmc1` cannot build `rmc2` until the candidate implements a real
  `build <path>` command.
- The real multi-file `rmc/main.rm` is not built by `rmc` yet.
- There is no full package lookup, stdlib lookup, HIR/MIR, LLVM backend, or
  fixed-point comparison pipeline.

## Artifact comparison

Generated C comparison was not valid for v0.0.9g because the chain stopped
before a real `rmc2` generation. Behavior was verified for `rmc1`, but behavior
could not be compared across `rmc1`, `rmc2`, and `rmc3`.

The attempted `rmc1 build tests/rmc_candidate/main.rm` produced:

```text
unknown command
```

That output is the blocker to address before exact generated-C comparison is
meaningful.

## Remaining gaps to v0.1.0

- Add a real `build <path>` command to the controlled candidate.
- Preserve candidate build outputs under distinct paths for staged comparison.
- Compare generated C and produced binary behavior across candidate generations.
- Expand real `rmc` source coverage beyond controlled fixtures.
- Add robust cross-module checking and diagnostics.
- Define deterministic artifact comparison rules.
- Harden regression coverage for the staged build chain.
- Keep docs and command-line build UX aligned with the actual verified chain.
