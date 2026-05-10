# RauMa v0.0.9 Fixed-Point Candidate

## What was verified

### Build chain

- `rmb` (bootstrap compiler) built `rmc0` (real RauMa-written `rmc`)
- `rmc0` built the controlled compiler candidate (`tests/rmc_candidate/main.rm`)
- The candidate binary (`rmc1`) was produced and runs all expected demo commands

### Behavior verification

`rmc1` passes the full candidate command suite:

- `version` prints `rmc-candidate 0.0.9f`
- `lex‑demo`, `parse‑demo`, `check‑demo`, `emit‑demo`, `build‑demo` produce the expected output sequences
- `self‑test` runs all module demos and ends with `candidate ok`
- `wat` (unknown command) prints `unknown command`

### Regression verification

All previous frontend groups, CLI/source/diag probes, `rmc‑mini`, and single‑file bridge tests still pass with `rmc0`. The `rmb` bootstrap compiler still builds `project_basic` and `build_hello` correctly.

## What this proves

- The RauMa-written `rmc` can compile a real‑ish compiler‑shaped module graph (the candidate) that contains nested module paths and transitive dependencies up to seven levels deep.
- The generated candidate binary behaves deterministically across runs.
- The local multi‑file bridge (`rmc build`) can handle a graph of this size without hitting earlier depth limits.

## What this does **not** prove (and why the chain stops here)

The candidate fixture currently **does not contain a `build` command** – it is a demo‑only program, not a full compiler. Consequently:

- `rmc1` cannot build `rmc2` because the candidate lacks the ability to compile source files.
- A true fixed‑point chain (`rmc0 → rmc1 → rmc2 → …`) therefore cannot be completed with the current candidate design.

The purpose of v0.0.9g was to **attempt** the chain; the attempt reveals that the candidate must be extended with a working `build` command before a real fixed‑point demonstration is possible.

## Artifact comparison

Because only `rmc1` exists, no cross‑generation comparison of generated C or binary behavior was possible. The candidate’s generated C (`build/rmc_build_out.c`) was saved but not compared across generations.

## Remaining gaps to v0.1.0

1. **Real `rmc` self‑host path** – the real `rmc/main.rm` is still not built by `rmc`.
2. **Candidate with `build` capability** – the candidate fixture would need to include a minimal `build` command that can compile its own source (or at least a trivial source) to close the loop.
3. **Deterministic artifact comparison** – once a chain of at least two generations exists, we need to compare generated C and binary outputs to verify stability.
4. **Robust cross‑module checking** – the current bridge still does not perform full type‑checking across module boundaries.
5. **Stronger regression suite** – the candidate should be expanded to exercise more of the real `rmc`’s source shapes (structs, arrays, error syntax, etc.).
6. **Docs and install/build UX** – the project still lacks end‑user documentation and a polished build/install story.

## Next steps

Given the outcome of v0.0.9g, two paths are reasonable:

**Option A (recommended)** – Extend the candidate with a minimal `build` command that can compile a trivial single‑file program (or the candidate itself) using the existing local multi‑file bridge. This would allow a true `rmc0 → rmc1 → rmc2` chain and prove that the bridge is stable across one generation.

**Option B** – Move directly to v0.1.0 stabilization, accepting that the fixed‑point candidate chain is blocked by the candidate’s design, and focus on polishing the existing bridge, improving diagnostics, and preparing for a release.

Both paths keep the real `rmc` self‑host goal separate, as it requires a much larger expansion of the bridge (package lookup, stdlib resolution, full type checking, HIR/MIR, etc.).

## Technical notes

- The candidate module graph contains 16 files across `cli/`, `source/`, `diag/`, `lex/`, `parse/`, `type/`, `cgen/`, and `build/` directories.
- The deepest transitive dependency chain is `main → build.build → cgen.cgen → type.checker → parse.parser → lex.lexer → source.source → source.span` (7 levels).
- The bridge’s module‑discovery algorithm (limited to direct dependencies plus one level of transitivity) still sufficed because each module’s own dependencies are shallow; a deeper graph would require the improved recursive discovery planned for v0.0.9f but not yet implemented.
- All verification was performed on Windows (MSYS2) with `gcc`; the `-Werror` flag is enabled, so the candidate builds cleanly with no unused‑function warnings.

---

*Generated during v0.0.9g verification, 2026‑05‑10*