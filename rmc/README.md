# RauMa Main Compiler (rmc)

## Status

`rmc` is the future main RauMa compiler written in RauMa.

v0.0.8o added the first emit-C bridge. `rmc emit-c <path>` reads a RauMa source
file with `read_file`, runs the lightweight checker, and emits simple C text for
tiny recognized source shapes.

v0.0.8p formalizes the **external compile workflow**: `rmc emit-c` writes C to
stdout, and an external C compiler (e.g. `gcc`) is responsible for turning that
C into an executable. `rmc` itself still does **not** invoke `gcc`, run
processes, or write files.

v0.0.8r adds an `rmc build <path>` bridge that wraps the emit-c → external-cc step
behind a single command. The command reads the file, checks it, emits C, writes
it to `build/rmc_build_out.c`, compiles it to `build/rmc_build_out` (or `.exe`
on Windows), and prints the result.

v0.0.8s expands that bridge subset. `rmc emit-c` and `rmc build` now recognize
several simple single-file integer/function patterns instead of only two fixed
templates.

v0.0.8t adds minimal bridge-style control flow. `rmc emit-c` and `rmc build`
now handle simple integer comparisons, assignment, `+=`, `if`/`else`, and
`while` for small single-file imperative programs. Multi-file/chunk builds,
HIR/MIR, packages, structs, and self-host remain future milestones.

v0.0.8u adds a tiny proto self-build target. The verified chain is `rmb` builds
the RauMa-written `rmc`, generated `rmc` builds `examples/selfbuild/tiny.rm`,
and the produced `build/rmc_build_out` executable runs. This is not
self-hosting yet: `rmc` does not build itself and there is no fixed point.

v0.0.8v adds a larger single-file CLI tool target at
`examples/selfbuild/tool.rm`. The same chain now proves `rmb` builds `rmc`,
`rmc build` compiles a compiler-tool-like RauMa program, and the produced tool
binary handles `version`, `score`, no-arg help, and unknown-command dispatch.
This extends the bridge subset with the minimal `Args`, `args_len`, `args_get`,
`str_eq`, void helper, and `return;` shapes needed by the target.

v0.0.8w consolidates the bridge emitter. The generated C prelude and function
wrappers are now split into smaller helpers, and new regression fixtures cover
Args dispatch, simple math calls, control flow, and `+=` emission so the bridge
subset is less fragile.

v0.0.8x audits self-build readiness in
`docs/compiler/self-build-readiness.md`. The current verified chain is still
`rmb` builds `rmc`, then `rmc` builds tiny/tool/probe single-file targets. `rmc`
does not build itself yet.

v0.0.8y adds a standalone pre-self-host target at
`examples/selfbuild/rmc-mini.rm`. It has no imports and is built by the
RauMa-written `rmc`; the produced binary behaves like a tiny compiler CLI with
`version`, `lex-demo`, `parse-demo`, and `check-demo` commands. This is still
not self-hosting.

v0.0.8z stabilizes the v0.0.8 bridge milestone. The supported bridge behavior,
verification chain, and fixed-point boundaries are documented in
`docs/compiler/v008-stabilization.md` and `docs/compiler/v009-plan.md`.

v0.0.9a hardens `examples/selfbuild/rmc-mini.rm`. The standalone target now
supports `version`, `lex-demo`, `parse-demo`, `check-demo`, and `self-test`.
It is still a single-file compiler-like target, not the real multi-file `rmc`.

v0.0.9b adds the first local multi-file foundation to `rmc build`. The command
can resolve simple local `use` modules next to the entry file, emit all modules
into one bridge C file, and compile it through the existing `cc_compile`
bridge. This is not a package manager, stdlib resolver, chunk cache, or
self-host fixed point.

v0.0.9c adds a real-ish `rmc` module group probe under
`rmb/tests/rmc_group_probe/`. The bridge now handles nested local module paths
such as `cli.help`, `source.span`, and `lex.token`, plus qualified calls across
that small graph. This proves the multi-file path can build compiler-shaped
module groups, but it is still not the full real `rmc` and not fixed-point
self-hosting.

v0.0.9d expands that probe into a broader `rmc-cli`-shaped module group at
`rmb/tests/rmc_cli_probe/`. Eight modules under `cli/`, `source/`, and `diag/`
exercise `Args` parameters in dependency functions, `path str` parameters,
`read_file`/`str_len`/`str_byte` builtins inside module functions, qualified
calls into nested namespaces, and diagnostic-style void helpers. The bridge
prelude now ships `rm_str_len`, `rm_str_byte`, and `rm_read_file` runtime
helpers so module code can read input files. This validates a broader
real-ish module topology, but `rmc` still does not build itself end-to-end and
there is no fixed point yet.

v0.0.9e adds real-ish frontend module group probes shaped like the real
compiler frontend:

- `rmb/tests/rmc_frontend_lexer/` — lexer-shaped graph (`source.span`,
  `source.source`, `diag.output`, `lex.char`, `lex.token`, `lex.lexer`)
  with `scan`, `invalid`, and `span` commands.
- `rmb/tests/rmc_frontend_parser/` — parser-shaped graph (`source.source`,
  `lex.token`, `parse.ast`, `parse.parser`, `diag.output`) with `parse` and
  `error` commands.
- `rmb/tests/rmc_frontend_checker/` — checker-shaped graph (`type.types`,
  `type.checker`, `diag.output`) with `check` and `mismatch` commands.
- `rmb/tests/rmc_frontend_combined/` — combined frontend graph that uses
  `source.*`, `lex.*`, `parse.*`, `type.*`, and `diag.output` together under
  one entry, with `lex`, `parse`, `check`, and `all` commands.

Each group exercises nested module paths, transitive local dependencies,
repeated module-name components across the graph, void module helpers, and
qualified calls into nested namespaces. The combined group glues
lexer/parser/checker modules into one buildable graph. This is still not real
`rmc` self-build, no fixed point, no package manager, no stdlib lookup, no
HIR/MIR.

v0.0.9f adds a controlled multi‑file `rmc` compiler candidate under
`rmb/tests/rmc_candidate/`. The candidate mirrors the real `rmc` compiler
module topology with `cli/`, `source/`, `diag/`, `lex/`, `parse/`, `type/`,
`cgen/`, and `build/` groups. Built with `rmc build`, the candidate binary
supports `version`, `lex‑demo`, `parse‑demo`, `check‑demo`, `emit‑demo`,
`build‑demo`, `self‑test`, and help/unknown‑command dispatch. This validates
the local multi‑file bridge with a deeper dependency graph (up to seven
levels) while still not reaching fixed‑point self‑hosting, package manager,
stdlib lookup, HIR/MIR, or real `rmc` self‑build.

The pipeline is now:

```bash
rmc build input.rm        # writes build/rmc_build_out.c, compiles it, prints status
./build/rmc_build_out     # runs the produced executable
```

Or for manual control:

```bash
rmc emit-c input.rm > out.c
gcc -std=c11 -Wall -Wextra -Werror -pedantic out.c -o out
./out
```

v0.0.9g verified the first step of a fixed‑point candidate chain: `rmb` builds `rmc0`, `rmc0` builds the candidate, and the candidate binary runs all demo commands. Because the candidate fixture does not contain a `build` command, the chain stops at one generation; a true fixed‑point demonstration awaits extending the candidate with a working `build` command.

`rmc` still does not allocate a full AST, infer types, resolve names, do full
codegen, implement HIR/MIR, packages, std modules, or self-host.

The current single-file bridge build subset supports:

- functions with `int` parameters/returns plus void helpers
- `fn main()` and `fn main(args Args)`
- `args_len`, `args_get`, and `str_eq` for command dispatch
- int locals from literals, calls, and simple `+` expressions
- one local string command variable from `args_get`
- assignment with `=` and `+=`
- `return expr;` and `return;`
- function call statements
- direct `print("...")` and `print(int_value)`
- integer comparisons with `==`, `!=`, `<`, `>`, `<=`, `>=`
- `if`/`else` and `while`

Still unsupported: general imports beyond local `use`, package/std lookup,
chunk layout in `rmc`, structs, arrays/slices, general string variables, string
concatenation, break/continue, match/defer/optional/error syntax, HIR/MIR, and
fixed-point self-hosting.

The local multi-file bridge subset supports:

- `use math;` -> `math.rm` beside the entry file
- `use cli.help;` -> `cli/help.rm` beside the entry file
- qualified calls such as `math.add(...)` and `cli.help.print_help()`
- nested local module paths such as `source.span` and `lex.token`
- one-level transitive local dependencies for small module groups
- one combined generated C file at `build/rmc_build_out.c`

There is no package lookup, stdlib lookup, robust cycle handling, chunk cache,
real module type checking, or per-module object layout in `rmc` yet.

## Current Source Layout

```text
rmc/
├── main.rm
├── cli/
│   ├── args.rm
│   ├── file.rm
│   ├── help.rm
│   └── version.rm
├── source/
│   ├── span.rm
│   └── source.rm
├── lex/
│   ├── token.rm
│   ├── lexer.rm
│   └── stream.rm
├── ast/
│   └── ast.rm
├── parse/
│   └── parser.rm
├── type/
│   └── checker.rm
├── cgen/
│   └── cgen.rm
└── diag/
    └── output.rm
```

`rmc/main.rm` imports the current internal modules:

```rauma
use cli.help;
use cli.version;
use cli.args;
use cli.file;
use lex.lexer;
use parse.parser;
use type.checker;
use cgen.cgen;
```

The executable dispatches commands through `fn main(args Args)`:

```text
help        print this help message
version     print compiler version
demo-lex    run hardcoded lexer demo
demo-parse  run hardcoded parser demo
demo-file   read a file and print basic info
lex         lex a RauMa source file
parse       parse a RauMa source file
check       check a RauMa source file
emit-c      emit C for a RauMa source file
build       build a RauMa source file (emit-c + write C + compile)
```

`demo-file <path>` only reads bytes and prints `file bytes` plus `first byte`.
`lex <path>` tokenizes the file contents only. `parse <path>` parses only the
current small subset and prints a summary using names from the source text.
`check <path>` performs lightweight structural validation only: parse success,
non-empty function set, and missing-return checks for functions with explicit
return types. `emit-c <path>` emits C to stdout only for tiny recognized
checked shapes. It does not compile or link generated C. The lexer and parser
demos still run against `source.source.demo_text()`.

The supported `rmc build` / `rmc emit-c` subset in v0.0.8s includes:

- direct `print("<string literal>")` in `main`
- local int variables initialized from int literals or simple function calls
- printing local int variables
- int functions with zero, one, or two int parameters
- `return <int literal>`
- `return <identifier>`
- `return <identifier> + <identifier>`
- simple calls with int literals, identifiers, or zero-argument calls as args
- assignment with `=`
- compound assignment with `+=`
- integer comparisons: `==`, `!=`, `<`, `>`, `<=`, `>=`
- `if`/`else`
- `while`

The bridge still emits C directly from cursor scans over the source text. It is
not a full backend, does not store a heap AST, and does not support
`break`/`continue`, bool variables, structs, string variables, multi-file
builds, or chunk layout in `rmc`.

The parser subset currently covers:

```text
file        = fn_decl* eof
fn_decl     = "fn" ident "(" param_list? ")" return_type? block
param_list  = param ("," param)*
param       = ident type_name
return_type = type_name
block       = "{" stmt* "}"
stmt        = return_stmt | var_stmt | assign_stmt | call_stmt | if_stmt | while_stmt
return_stmt = "return" expr ";"
var_stmt    = ident ":=" expr ";"
assign_stmt = ident ("=" | "+=") expr ";"
call_stmt   = ident "(" arg_list? ")" ";"
if_stmt     = "if" "(" expr compare expr ")" block ("else" block)?
while_stmt  = "while" "(" expr compare expr ")" block
expr        = call_expr | primary ("+" primary)?
primary     = ident | int | string
```

## Building With rmb

From `rmb/`:

```bash
make
./build/rmb build ../rmc/main.rm
./build/debug/native/bin/main
```

The binary path is still entry-stem based, so `rmc/main.rm` builds to
`rmb/build/debug/native/bin/main`.

## Proto Self-Build Chain

From `rmb/`:

```bash
make
./build/rmb build ../rmc/main.rm
./build/debug/native/bin/main build ../examples/selfbuild/tiny.rm
./build/rmc_build_out
```

Expected output:

```text
tiny self-build
ok
42
```

The test fixture `rmb/tests/rmc_selfbuild_tiny.rm` contains the same source for
stable verification inside the bootstrap test tree.

## External Compile Workflow

The `rmb/tests/rmc_emit_workflow_*.rm` fixtures verify the bridge end-to-end.
From `rmb/`:

```bash
./build/rmb build ../rmc/main.rm
./build/debug/native/bin/main emit-c tests/rmc_emit_workflow_hello.rm > build/rmc_emit_workflow_hello.c
gcc -std=c11 -Wall -Wextra -Werror -pedantic build/rmc_emit_workflow_hello.c -o build/rmc_emit_workflow_hello
./build/rmc_emit_workflow_hello   # prints: hello from rmc
```

```bash
./build/debug/native/bin/main emit-c tests/rmc_emit_workflow_add.rm > build/rmc_emit_workflow_add.c
gcc -std=c11 -Wall -Wextra -Werror -pedantic build/rmc_emit_workflow_add.c -o build/rmc_emit_workflow_add
./build/rmc_emit_workflow_add     # prints: 42
```

```bash
./build/debug/native/bin/main emit-c tests/rmc_emit_workflow_fail.rm
# prints: emit failed: check failed
# (no C is emitted, so nothing is compiled)
```

If `make` is available, `make test-rmc-emit-workflow` runs the same pipeline.

### PowerShell redirection caveat

Native PowerShell `>` redirection writes UTF-16 by default, which `gcc` cannot
parse. On Windows, use bash (the redirection above is plain `>`), or invoke via
`cmd /c "... > out.c"` so the output stays as plain bytes.

## v0.0.8 Stabilized Targets

The stabilized proto self-build targets are:

- `examples/selfbuild/tiny.rm`
- `examples/selfbuild/tool.rm`
- `examples/selfbuild/rmc-mini.rm`

`rmb` builds `rmc`, and `rmc` builds each target through the single-file bridge.
The real multi-file `rmc` compiler is still not built by `rmc`; fixed-point
self-hosting remains future v0.0.9 work.

## rmc-mini Commands

`examples/selfbuild/rmc-mini.rm` is standalone and import-free. When built by
`rmc build`, the produced binary supports:

```text
version
lex-demo
parse-demo
check-demo
self-test
```

The `self-test` command exercises helper calls, loops, conditionals, and int
printing. It is only a pre-self-host target, not the real `rmc`.
