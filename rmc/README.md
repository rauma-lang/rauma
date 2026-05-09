# RauMa Main Compiler (rmc)

## Status

`rmc` is the future main RauMa compiler written in RauMa.

v0.0.8o added the first emit-C bridge. `rmc emit-c <path>` reads a RauMa source
file with `read_file`, runs the lightweight checker, and emits simple C text for
tiny recognized source shapes.

v0.0.8p formalizes the **external compile workflow**: `rmc emit-c` writes C to
stdout, and an external C compiler (e.g. `gcc`) is responsible for turning that
C into an executable. `rmc` itself still does **not** invoke `gcc`, run
processes, or write files. The pipeline is:

```bash
rmc emit-c input.rm > out.c
gcc -std=c11 -Wall -Wextra -Werror -pedantic out.c -o out
./out
```

`rmc` still does not allocate a full AST, infer types, resolve names, do full
codegen, compile or link generated C, implement HIR/MIR, packages, std modules,
or self-host.

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
```

`demo-file <path>` only reads bytes and prints `file bytes` plus `first byte`.
`lex <path>` tokenizes the file contents only. `parse <path>` parses only the
current small subset and prints a summary using names from the source text.
`check <path>` performs lightweight structural validation only: parse success,
non-empty function set, and missing-return checks for functions with explicit
return types. `emit-c <path>` emits C to stdout only for tiny recognized
checked shapes. It does not compile or link generated C. The lexer and parser
demos still run against `source.source.demo_text()`.

The recognized shapes for `emit-c` in v0.0.8p are:

```rauma
fn main() {
    print(<string-literal>);
}
```

and

```rauma
fn add(a int, b int) int {
    return a + b;
}

fn main() {
    x := add(<int-literal>, <int-literal>);
    print(x);
    print(<string-literal>);
}
```

The literal text is copied verbatim into the emitted C, so the runtime output
of the compiled binary reflects the source.

The parser subset currently covers:

```text
file        = fn_decl* eof
fn_decl     = "fn" ident "(" param_list? ")" return_type? block
param_list  = param ("," param)*
param       = ident type_name
return_type = type_name
block       = "{" stmt* "}"
stmt        = return_stmt | var_stmt | call_stmt
return_stmt = "return" expr ";"
var_stmt    = ident ":=" expr ";"
call_stmt   = ident "(" arg_list? ")" ";"
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

v0.0.8q will add an `rmc build` bridge that wraps the emit-c → external-cc step
behind a single command. Self-host fixed point remains v0.0.9.
