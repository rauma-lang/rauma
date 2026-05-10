# RauMa v0.0.9 Real Self-Host Expansion

## Status

v0.0.9h attempts to move from the controlled `rmc_candidate` fixed-point chain
to the real `rmc/main.rm` source graph.

The v0.0.9h-fix2 retry still verifies Tier 0:

- `rmb` still builds `rmc0`.
- `rmc0 version` and `rmc0 help` still work.
- `rmc0 build ../rmc/main.rm` still fails before producing `rmc1-real`.
- the focused boolean/logical bridge fixture builds and runs.

This is not real `rmc` self-hosting.

## What changed

The real `rmc` source was trimmed toward the current bridge subset:

- removed unused data declaration shells from `rmc/source`, `rmc/ast`, and
  `rmc/lex/token`
- taught the `rmc` lexer bridge about `pub`, `struct`, `enum`, `!`, `&&`, and
  `||` tokens
- skipped `pub` modifiers and line comments during token scanning
- allowed bridge C emission to accept `bool` return annotations as an integer
  bridge type
- removed the `source.source.demo_text() str` helper by inlining demo source
  text at its call sites
- v0.0.9h-fix2 adds bridge C lowering for comparison expressions, logical
  `&&`, logical `||`, and parenthesized boolean expressions in
  `rmc/cgen/cgen.rm`
- `rmb` build chunks now use a larger arena so the expanded real `rmc/cgen`
  source can still be bootstrapped by the C bootstrap compiler

## Highest tier reached

Tier 0 only.

Tier 1 requires:

```text
rmc0 build ../rmc/main.rm
```

to produce `build/rmc_build_out`. In v0.0.9h-fix2 it still reports:

```text
build failed: unsupported source
```

No `rmc1-real` was produced, so Tier 1, Tier 2, Tier 3, and Tier 4 were not
reached.

## Current blocker

The previous first isolated unsupported source shape was in `rmc/lex/lexer.rm`.

The real lexer uses expressions such as:

```rauma
if (b >= 65 && b <= 90) {
```

and:

```rauma
return b >= 48 && b <= 57;
```

v0.0.9h-fix2 lowers those shapes. The focused fixture at
`rmb/tests/rmc_bool_bridge/main.rm` now builds and runs with:

```text
upper ok
lower ok
alpha ok
inline and ok
inline or ok
```

The emitted C contains `>=`, `<=`, `&&`, and `||`.

The next visible blocker is no longer this boolean bridge fixture. The real
retry still fails before `rmc1-real`: `rmc0 build ../rmc/main.rm` reports
`build failed: unsupported source`, while direct `rmc0 parse/check
../rmc/main.rm` reports `parser error`. The next milestone should isolate that
remaining real-source parser/codegen shape with a small fixture.

## What this does not prove

- real `rmc` does not build itself yet
- no `rmc1-real`, `rmc2-real`, or `rmc3-real` was produced
- no real fixed-point comparison was possible
- the controlled candidate chain remains separate from real compiler
  self-hosting

## Recommended next step

Run a focused follow-up that isolates the next parser/unsupported-source shape
behind `rmc0 build ../rmc/main.rm`, preferably with a small targeted fixture
before retrying the real self-host chain again.
