# RauMa Main Compiler (rmc)

## Status

`rmc` is the future main RauMa compiler written in RauMa.

v0.0.8e adds a minimal parser foundation over the same hardcoded demo source.
The parser uses byte-level source scanning with `str_len` and `str_byte`,
recognizes only the demo grammar, and prints a stable parser summary.

`rmc` still does not implement file IO, CLI args, token arrays, full parsing,
checking, codegen, HIR, MIR, packages, std modules, or self-hosting.

## Current Source Layout

```text
rmc/
├── main.rm
├── cli/
│   ├── help.rm
│   └── version.rm
├── source/
│   ├── span.rm
│   └── source.rm
├── lex/
│   ├── token.rm
│   └── lexer.rm
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
use source.source;
use lex.token;
use lex.lexer;
use ast.ast;
use parse.parser;
use diag.output;
```

The executable prints help, module readiness, lexer smoke tokens, and a parser
summary scanned from `source.source.demo_text()`. Command-line argument
dispatch is not implemented yet because the current bootstrap codegen does not
support `main(args)`.

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

v0.0.8f should introduce a token stream abstraction. File-driven lexing,
full parser work, checker/codegen bridge work, and the self-host fixed point
remain later milestones.
