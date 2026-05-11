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

## v0.0.9h-fix3 blocker sprint

v0.0.9h-fix3 fixes five focused bridge blockers with fixtures under
`rmb/tests/rmc_real_blockers/`:

- `blocker_01_use_qualified_call`: top-level `use` declarations and qualified
  call statements in the real parser
- `blocker_02_unary_additive_compare`: parenthesized expressions, unary `!`,
  and additive expressions inside comparisons
- `blocker_03_str_return`: bridge C emission for `str` return functions
- `blocker_04_bool_param`: bridge C emission for `bool` parameters
- `blocker_05_string_builtins`: string slice/concat/print-slice runtime calls
  plus string-like local variable emission

The focused fixtures all build and run. The controlled candidate chain still
passes through `rmc3`, and the previous frontend/probe/mini/bool regressions
still pass.

The real `rmc/main.rm` retry progresses further than fix2:

```text
rmc0 parse ../rmc/main.rm  -> pass
rmc0 check ../rmc/main.rm  -> pass
rmc0 build ../rmc/main.rm  -> build failed: cc failed
```

The highest real self-host tier remains Tier 0 because no `rmc1-real` is
produced. The next blocker is now generated-C correctness for the real
multi-module graph: unresolved cross-module forward references, duplicate
`rm_fn_*` compatibility macros across modules, and remaining string-aware
local type inference/printing gaps in generated C.

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
