# RauMa Main Compiler (rmc)

## Status

`rmc` is the future main RauMa compiler written in RauMa.

v0.0.8c adds the first `rmc` lexer module and wires a lexer smoke demo into the
binary. The current lexer module is a deterministic smoke-output placeholder
over the fixed demo source shape, not a real scanner yet. Real string scanning
waits for later source/string indexing and file input support.

`rmc` still does not implement file IO, CLI args, parsing, checking, codegen,
HIR, MIR, packages, std modules, or self-hosting.

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
use diag.output;
```

The executable prints help, a small module readiness section, and a stable
lexer smoke token list. Command-line argument dispatch is not implemented yet
because the current bootstrap codegen does not support `main(args)`.

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

v0.0.8d should add the parser foundation. File-driven lexing, checker/codegen
bridge work, and the self-host fixed point remain later milestones.
