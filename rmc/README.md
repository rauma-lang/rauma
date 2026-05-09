# RauMa Main Compiler (rmc)

## Status

`rmc` is the future main RauMa compiler written in RauMa.

v0.0.8n adds a lightweight file-driven checker. `rmc check <path>` reads a
RauMa source file with `read_file`, reuses the parser foundation, and performs
small structural checks for the supported subset.

`rmc` still does not allocate a full AST, infer types, resolve names, codegen,
implement HIR/MIR, packages, std modules, or self-host.

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
```

`demo-file <path>` only reads bytes and prints `file bytes` plus `first byte`.
`lex <path>` tokenizes the file contents only. `parse <path>` parses only the
current small subset and prints a summary using names from the source text.
`check <path>` performs lightweight structural validation only: parse success,
non-empty function set, and missing-return checks for functions with explicit
return types. It does not perform type inference, name resolution, or codegen.
The lexer and parser demos still run against `source.source.demo_text()`.

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

## Later v0.0.8 Work

v0.0.8o should start the emit-C bridge. The self-host fixed point
remains a later milestone.
