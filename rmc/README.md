# RauMa Main Compiler (rmc)

## Status

`rmc` is the future main RauMa compiler written in RauMa.

v0.0.8a is only the first skeleton. It proves that `rmb` can build a
RauMa-written `rmc` binary with multiple source chunks. It does not contain the
real lexer, parser, checker, codegen, driver, HIR, MIR, package system, or
self-hosting logic yet.

## Current Source Layout

```text
rmc/
├── main.rm
└── cli/
    ├── help.rm
    └── version.rm
```

`rmc/main.rm` imports internal CLI modules:

```rauma
use cli.help;
use cli.version;
```

The current executable prints help from `cli.help`. `cli.version` exists as the
initial version module, but command-line argument dispatch is not implemented
yet because the current bootstrap codegen does not support `main(args)`.

## Building With rmb

From `rmb/`:

```bash
make
./build/rmb build ../rmc/main.rm
./build/debug/native/bin/main
```

The binary path is still entry-stem based in v0.0.7/v0.0.8a, so
`rmc/main.rm` builds to `rmb/build/debug/native/bin/main`.

## Later v0.0.8 Work

Later v0.0.8 steps will add RauMa-written source/span/token modules, then the
lexer, parser foundation, and checker/codegen bridge. The self-host fixed point
remains a separate v0.0.9 milestone.
