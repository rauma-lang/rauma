# RauMa Main Compiler (rmc)

## Status

`rmc` is the future main RauMa compiler written in RauMa.

v0.0.8i adds a file input smoke command through the temporary `read_file`
builtin. The `rmc` binary can now read a file and print basic byte information,
but it still does not lex or parse file contents.

`rmc` still does not implement full file-driven lexing, token arrays, full AST
allocation, checking, codegen, HIR, MIR, packages, std modules, or self-hosting.

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
```

`demo-file <path>` only reads bytes and prints `file bytes` plus `first byte`.
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

v0.0.8j should add a real lex-file demo command. Full parser work,
checker/codegen bridge work, and the self-host fixed point remain later
milestones.
