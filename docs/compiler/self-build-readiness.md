# RauMa Self-Build Readiness

## Current status

The current bootstrap chain is:

- `rmb` builds the RauMa-written `rmc`.
- `rmc` can lex, parse, check, emit C for, and build small single-file RauMa programs.
- `rmc` can build `examples/selfbuild/tiny.rm`.
- `rmc` can build `examples/selfbuild/tool.rm`.
- `rmc` can build `examples/selfbuild/rmc-mini.rm`.
- v0.0.9a hardens `rmc-mini` to `rmc-mini 0.0.9a` and adds a
  `self-test` command.
- v0.0.9b starts local multi-file `rmc build` by resolving simple local `use`
  modules and emitting one combined bridge C file.
- v0.0.9c builds a real-ish `rmc` module group probe with nested `cli`,
  `source`, and `lex`-style modules.
- v0.0.9d builds a larger `rmc-cli`-shaped module group probe at
  `rmb/tests/rmc_cli_probe/` with eight modules across `cli/`, `source/`, and
  `diag/`. It exercises `Args`/`path str` parameters in dependency module
  functions, `read_file`/`str_len`/`str_byte` builtins inside module
  functions, qualified calls into nested namespaces, and diagnostic-style
  void helpers.
- v0.0.9e builds real-ish frontend module group probes shaped like the real
  compiler frontend at `rmb/tests/rmc_frontend_lexer/`,
  `rmb/tests/rmc_frontend_parser/`, `rmb/tests/rmc_frontend_checker/`, and
  `rmb/tests/rmc_frontend_combined/`. Each group exercises nested module
  paths (`source.span`, `lex.token`, `parse.parser`, `type.checker`),
  transitive local dependencies, repeated module-name components across the
  graph, void module helpers, and qualified calls into nested namespaces.
  The combined group glues lexer/parser/checker modules into one buildable
  graph.
- v0.0.9f adds a controlled multi‑file `rmc` compiler candidate under
  `rmb/tests/rmc_candidate/` with `cli/`, `source/`, `diag/`, `lex/`, `parse/`,
  `type/`, `cgen/`, and `build/` module groups. The candidate binary supports
  `version`, `lex‑demo`, `parse‑demo`, `check‑demo`, `emit‑demo`, `build‑demo`,
  `self‑test`, and help/unknown‑command dispatch. It validates the local
  multi‑file bridge with a deeper dependency graph (up to seven levels).
- v0.0.9g-fix verifies the controlled fixed‑point candidate chain:
  `rmb -> rmc0 -> rmc1 -> rmc2 -> rmc3` for
  `rmb/tests/rmc_candidate/main.rm`. Candidate behavior matches across
  generations, and generated C from `rmc1` and `rmc2` matches exactly.
  This is still not real `rmc` self-hosting because the candidate build command
  is path-specific and does not build the full real `rmc/main.rm`.
- v0.0.8z stabilizes the bridge milestone and points fixed-point work to
  `docs/compiler/v009-plan.md`.
- This is not self-hosting yet.

`rmc build` is currently a bridge: it reads a single file, runs lightweight checks, emits C text, writes `build/rmc_build_out.c`, invokes the external C compiler through the temporary `cc_compile` builtin, and produces `build/rmc_build_out`.

## Verified bridge subset

The verified bridge subset currently includes:

- single-file source
- small local multi-file source through `use`
- `fn`
- typed int parameters
- int return values
- `fn main()`
- `fn main(args Args)`
- local int variables
- local string command variable from `args_get`
- assignment
- compound assignment
- `return expr`
- `return;`
- direct function calls
- `print(string literal)`
- `print(int variable)`
- integer literals
- string literals in direct print
- `+`
- integer comparisons
- `if/else`
- `while`
- `Args`, `args_len`, `args_get`
- `str_eq`
- `read_file`, `write_file`, `cc_compile` as bridge builtins
- local `use math;` / `use cli.help;` resolution relative to the entry file
- nested local modules such as `source.span` and `lex.token`
- small transitive local module graphs used by the `rmc_group_probe` fixture
- broader CLI/source/diag module groups used by the `rmc_cli_probe` fixture
- `Args` and `path str` parameters in dependency module functions
- `read_file`, `str_len`, `str_byte` builtins inside dependency module
  functions
- string literal arguments wrapped through `rm_str` for qualified calls
- `return;` in module void functions emits valid C

## Current rmc source feature audit

| Area | Files | Uses | Buildable today? | Blockers |
| --- | --- | --- | --- | --- |
| CLI dispatch | `rmc/main.rm`, `rmc/cli/help.rm`, `rmc/cli/version.rm`, `rmc/cli/args.rm`, `rmc/cli/file.rm` | imports, `main(args Args)`, command dispatch, file reads, string/int prints, many helper calls | Partial | `use` imports and multi-file build are not supported by `rmc build`; `read_file` is usable but command modules cannot be linked by `rmc` yet |
| Source/span/token declarations | `rmc/source/span.rm`, `rmc/source/source.rm`, `rmc/lex/token.rm` | `struct`, `enum`, module imports, many small functions, token-kind constants | No | bridge codegen has no struct/enum lowering for `rmc` output, no imports, no module/chunk layout |
| Lexer | `rmc/lex/lexer.rm` | byte scanning with `str_len`/`str_byte`, many helper functions, long if chains, loops, token printing | Partial | single-file extraction may work for small pieces, but current file imports modules and depends on token/source modules; broader helper surface exceeds tested bridge shape |
| Token stream | `rmc/lex/stream.rm` | cursor helpers, token text slices, byte/text comparison helpers, imports lexer/token/source | Partial | depends on imports and module linking; uses string slice helper and many cross-module calls |
| AST summary | `rmc/ast/ast.rm` | tiny print helpers | Partial | standalone print-only content is easy, but current module participates via imports |
| Parser | `rmc/parse/parser.rm` | recursive descent helpers, counters, nested parse calls, token stream helpers, summary printing | No | heavy cross-module dependency on stream/token/source, no token arrays/AST storage, no module build, bridge parser/codegen only covers smaller user-program patterns |
| Checker | `rmc/type/checker.rm` | parser helper calls and check summary output | No | depends on parser module and has no real symbol/type model yet |
| C emitter | `rmc/cgen/cgen.rm` | token-stream driven C emission, dynamic string concatenation, builtins, many helper functions | No | depends on checker/stream/token, relies on `str_concat`/`str_from_slice`, and is broader than current `rmc build` subset |
| Build bridge | `rmc/build/build.rm` | `write_file`, `cc_compile`, emits fixed output path | Partial | could be mimicked standalone, but real module imports checker/cgen and needs module/chunk support |
| Diagnostics | `rmc/diag/output.rm` | small print helper | Partial | buildable only as standalone; real module linking is missing |

## Main gaps before rmc can build itself

- larger CLI/source/diag module group is now verified (v0.0.9d)
- real-ish frontend-shaped module groups (lexer, parser, checker, combined)
  are now verified (v0.0.9e)
- real `rmc` candidate compiler build through `rmc build` (next: v0.0.9f)
- deterministic fixed-point comparison strategy (still later: v0.0.9g)
- broader real `rmc` source coverage beyond controlled module-group probes
- robust module type checking in the bridge (parser/checker is bypassed for
  multi-module builds today)
- chunk cache and per-module incremental rebuild
- deterministic fixed-point comparison strategy
- broader multi-file `use` resolution in `rmc build`
- real module/chunk build in `rmc`
- more general parser/codegen instead of bridge patterns
- token arrays or better cursor abstraction
- heap AST or structured summary model
- broader string handling
- general function emission
- general statement emission
- general expression emission
- better name resolution
- real module-aware type checking
- better error handling
- output paths per source
- support for more `rmc` source shapes, especially structs, enums, imports, cross-module calls, and large helper-heavy files
- deterministic fixed-point comparison strategy

## Candidate next self-build targets

The v0.0.8y milestone verified the recommended reduced `rmc-mini.rm` target. It
proves that the bridge can build a standalone compiler-like CLI with version,
lexer-demo, parser-demo, and check-demo commands.

1. Build a single extracted CLI module as standalone source.

Pros: closest to existing `rmc` command dispatch and mostly uses the verified `Args` subset.  
Cons: not representative of lexer/parser/compiler internals unless imports are removed.

2. Build a standalone lexer smoke program.

Pros: exercises byte scanning, loops, token classification, and compiler-like behavior.  
Cons: needs either copied token helpers or real module support; the current lexer has many cross-module dependencies.

3. Build a standalone parser smoke program.

Pros: closer to compiler logic and validates cursor parsing.  
Cons: parser depends heavily on token stream helpers and is much larger than current bridge confidence.

4. Build a reduced `rmc-mini.rm`. ✓ verified in v0.0.8y and hardened in v0.0.9a

Pros: best next bridge target because it can avoid imports while mimicking a tiny compiler CLI with version, lex-demo, parse-demo, check-demo, and self-test commands.  
Cons: duplicates a thin slice of `rmc` behavior and is still not self-hosting.

## Recommended next milestone

v0.0.9a hardens the standalone `rmc-mini.rm` target that mimics a tiny compiler command:

- `main(args Args)`
- `version`
- `lex-demo`
- `parse-demo`
- `check-demo`
- `self-test`
- no imports
- no multi-file
- buildable by `rmc build`

v0.0.9e adds real-ish frontend module group probes (lexer, parser, checker,
combined) under `rmb/tests/rmc_frontend_*/`. They exercise the bridge subset
across nested namespaces and transitive local dependencies without rewriting
the real frontend and without claiming fixed point.

The next milestone should be v0.0.9f: a real `rmc` compiler candidate built
through `rmc build`. The remaining compressed v0.0.9 path is:

1. v0.0.9e — real-ish frontend module groups (this milestone)
2. v0.0.9f — rmc compiler candidate
3. v0.0.9g — fixed-point candidate

## Not yet self-hosting

v0.0.9e is not self-hosting.

`rmc` does not build itself yet. `rmc` does not have multi-file chunk builds, HIR/MIR, a full backend, or fixed-point verification. v0.0.9 remains the self-host fixed-point milestone.
