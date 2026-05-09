# RauMa Main Compiler (rmc)

## Status

`rmc` is the future main RauMa compiler written in RauMa.

v0.0.8k adds the first file-driven parser command. `rmc parse <path>` reads a
RauMa source file with `read_file` and parses the current small demo subset
through token stream helpers.

`rmc` still does not allocate a full AST, check, codegen, implement HIR/MIR,
packages, std modules, or self-host.

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
```

`demo-file <path>` only reads bytes and prints `file bytes` plus `first byte`.
`lex <path>` tokenizes the file contents only; it does not parse, typecheck, or
generate code. `parse <path>` parses only the current small subset and prints a
fixed parser summary for that shape; it does not typecheck or generate code.
The lexer and parser demos still run against `source.source.demo_text()`.

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

v0.0.8l should start the checker/codegen bridge or continue parser
generalization. The self-host fixed point remains a later milestone.
