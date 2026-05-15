# RauMa LSP Protocol

RauMa v0.2.0 includes a bootstrap LSP module group under `rmc/lsp/`.

## Commands

```bash
rmc lsp --check-exits-immediately
rmc lsp --capabilities
rmc lsp
```

`--check-exits-immediately` is intended for CI. It verifies that the LSP module
graph is linked into the compiler and can be dispatched without blocking.

`--capabilities` prints the JSON-RPC initialize result body. `rmc lsp` prints a
framed initialize response through the current bridge stdout path.

## Capabilities

- full text document sync
- completion provider
- hover provider
- definition provider

The current implementation exposes protocol helpers, keyword/builtin completion
data, minimal document helpers, and dispatch smoke paths. Full stdin JSON-RPC
message parsing is still blocked on stable bridge stdin/runtime support.

## Module Layout

```text
rmc/lsp/
  protocol.rm
  message.rm
  docs.rm
  handlers.rm
  dispatch.rm
```

The modules are bridge-compatible and are included from `rmc/main.rm`.
