# RauMa Main Compiler (rmc)

## Status

`rmc` is the future main RauMa compiler written in RauMa.

v0.0.8h adds command dispatch through the bootstrap `Args` type. The `rmc`
binary now supports help, version, lexer demo, and parser demo commands while
the demo source remains hardcoded.

`rmc` still does not implement file IO, token arrays, full AST
allocation, checking, codegen, HIR, MIR, packages, std modules, or self-hosting.

## Current Source Layout

```text
rmc/
├── main.rm
├── cli/
│   ├── args.rm
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
use lex.lexer;
use parse.parser;
```

The executable dispatches commands through `fn main(args Args)`:

```text
help        print this help message
version     print compiler version
demo-lex    run hardcoded lexer demo
demo-parse  run hardcoded parser demo
```

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

v0.0.8i should prepare file input. File-driven lexing,
full parser work, checker/codegen bridge work, and the self-host fixed point
remain later milestones.
