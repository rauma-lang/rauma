# RauMa v0.0.9 Real Self-Host Expansion

## Status

v0.0.9h attempts to move from the controlled `rmc_candidate` fixed-point chain
to the real `rmc/main.rm` source graph.

The verified result is Tier 0:

- `rmb` still builds `rmc0`.
- `rmc0 version` and `rmc0 help` still work.
- `rmc0 build ../rmc/main.rm` still fails before producing `rmc1-real`.

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

## Highest tier reached

Tier 0 only.

Tier 1 requires:

```text
rmc0 build ../rmc/main.rm
```

to produce `build/rmc_build_out`. In v0.0.9h it still reports:

```text
build failed: unsupported source
```

## Current blocker

The first isolated unsupported source shape is in `rmc/lex/lexer.rm`.

The real lexer uses expressions such as:

```rauma
if (b >= 65 && b <= 90) {
```

and:

```rauma
return b >= 48 && b <= 57;
```

The current bridge emitter can tokenize `&&`, `||`, and `!`, but its supported
expression emission is still too small to lower general boolean/logical
expressions safely. A direct attempt to add that support to `rmc/cgen/cgen.rm`
made the bootstrap `rmb build ../rmc/main.rm` path unstable, so it was not kept.

## What this does not prove

- real `rmc` does not build itself yet
- no `rmc1-real`, `rmc2-real`, or `rmc3-real` was produced
- no real fixed-point comparison was possible
- the controlled candidate chain remains separate from real compiler
  self-hosting

## Recommended next step

Run a focused v0.0.9h-fix milestone that isolates boolean/logical expression
lowering for the bridge emitter, preferably with a small targeted fixture before
retrying `rmc0 build ../rmc/main.rm`.

