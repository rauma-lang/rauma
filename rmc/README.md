# RauMa Main Compiler (rmc)

## Status

`rmc` is the future main RauMa compiler written in RauMa.

v0.0.8b adds the first compiler data modules: source/span shapes, token kind
shapes, token helpers, and tiny diagnostic output helpers. It still does not
implement lexing, parsing, checking, codegen, HIR, MIR, packages, std modules,
or self-hosting.

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
│   └── token.rm
└── diag/
    └── output.rm
```

`rmc/main.rm` imports the current internal modules:

```rauma
use cli.help;
use cli.version;
use source.source;
use lex.token;
use diag.output;
```

The executable prints help and a small module readiness section. Command-line
argument dispatch is not implemented yet because the current bootstrap codegen
does not support `main(args)`.

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

v0.0.8c should add the RauMa-written lexer. Parser foundation,
checker/codegen bridge work, and the self-host fixed point remain later
milestones.
