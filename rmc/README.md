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

Still unsupported: imports, multi-file builds, chunk layout in `rmc`, structs,
arrays/slices, general string variables, string concatenation, break/continue,
match/defer/optional/error syntax, HIR/MIR, and fixed-point self-hosting.

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

## Later v0.0.8 Work

v0.0.8z should stabilize v0.0.8 and prepare the transition toward v0.0.9
fixed-point planning. Self-host fixed point remains v0.0.9.
