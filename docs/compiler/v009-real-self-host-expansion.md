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

## v0.0.9h-fix4 generated-C graph sprint

v0.0.9h-fix4 fixes the generated-C graph blockers that kept the real build at
`build failed: cc failed`.

The fixes are intentionally bridge-scoped:

- emit C prototypes before function bodies for same-file and same-module calls
- scope `rm_fn_*` compatibility macros inside each emitted module, then `#undef`
  them before the next module
- emit nested dependency modules deeply enough for the current real graph, so
  `source.source` sees `source.span`
- preserve generated helper functions under `-Werror` by marking bridge-emitted
  static functions as unused-capable
- extend string-aware emission so `RmStr` locals and prints are not lowered as
  integer `printf("%lld", ...)`

The real chain now reaches Tier 4:

```text
rmc0 build ../rmc/main.rm  -> build ok: build/rmc_build_out
rmc1-real version          -> rmc 0.0.9h-fix
rmc1-real help             -> pass
rmc1-real build ../rmc/main.rm -> build ok: build/rmc_build_out
rmc2-real build ../rmc/main.rm -> build ok: build/rmc_build_out
rmc3-real version/help     -> pass
```

Tier 2 smoke also passed for `demo-lex`, `demo-parse`, `lex`, `parse`, `check`,
`emit-c`, candidate build, and candidate self-test. The controlled candidate
chain still reaches `rmc3`, and the previous frontend/probe/mini/bool
regressions plus `make test` still pass.

## What this does not prove

- real fixed-point artifact comparison is not implemented yet
- the controlled candidate chain remains separate from real compiler
  self-hosting
- module dependency emission is still a bridge implementation, not a package
  manager or general stdlib lookup

## Recommended next step

The immediate next step after fix4 was deterministic C and behavior comparison
for the real `rmc1-real`, `rmc2-real`, and `rmc3-real` outputs.

## v0.0.9i deterministic real self-host verification

v0.0.9i rebuilds the real chain and verifies deterministic generated C across
the self-host stages:

```text
rmb -> rmc0
rmc0 -> rmc1-real
rmc1-real -> rmc2-real
rmc2-real -> rmc3-real
```

The generated C artifacts matched exactly:

```text
real_from_rmc0.c 34e3bfe394347d852aa34db5dc753f6f22e63ed1a119d6d54d57b079da93db27 207067
real_from_rmc1.c 34e3bfe394347d852aa34db5dc753f6f22e63ed1a119d6d54d57b079da93db27 207067
real_from_rmc2.c 34e3bfe394347d852aa34db5dc753f6f22e63ed1a119d6d54d57b079da93db27 207067
generated_c_compare= PASS
```

Behavior comparison across `rmc1-real`, `rmc2-real`, and `rmc3-real` also
passed for no-args, version, help, demo lex/parse, lex, parse, check, emit-c,
candidate build plus candidate self-test, and `wat`.

No deterministic source fix was needed. The remaining limitation is release
hardening: the architecture is still bridge-based and still lacks package
lookup, stdlib lookup, HIR/MIR, LLVM, and release polish.

## v0.1.0 release result

v0.1.0 turns the verified real self-host chain into release automation. CI runs
the deterministic self-host verification, and the release workflow publishes:

- `rmc-windows-x64-gcc.exe`
- `rmb-windows-x64-gcc.exe`
- `rauma-setup.sh`
- `rauma-setup.ps1`
- `rauma-v0.1.0-windows-x64-gcc.zip`
- `SHA256SUMS.txt`

`rmc` now reports `rmc 0.1.0`. The release remains bridge-based and local
module graph only; package management, stdlib lookup, HIR/MIR, LLVM, rmgen, and
rmlink remain future work.
