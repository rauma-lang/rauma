# Bootstrap Process

## Overview

RauMa's bootstrap process enables the compiler to build itself, ensuring the language implementation is correct and consistent.

## Bootstrap Stages

### Stage 0: rmb (Bootstrap Compiler)
- Written in C11
- Minimal RauMa compiler
- Only implements core language subset
- Generates C code as output
- Stable, verified implementation
- In v0.0.8a, still the active bootstrap compiler and the tool used to build
  the first `rmc` skeleton.

### Stage 1: rmc1 (First RauMa Compiler)
- Written in RauMa bootstrap subset
- Compiled by rmb
- Implements full RauMa language
- Can compile RauMa source to C
- More features than rmb

### v0.0.8a: Initial rmc Skeleton

v0.0.8a starts `rmc` as a RauMa-written binary under `rmc/`. This binary is not
a real compiler yet. It only establishes the source layout, imports internal
modules, and prints version/help text through a binary built by `rmb`.

The self-host fixed point is still v0.0.9. Until then, `rmb` remains the
bootstrap compiler and source of truth for compiling the early `rmc` code.

### v0.0.8b: Compiler Data Modules

v0.0.8b adds the first compiler data modules in RauMa:

- source/span data shapes
- token kind and token data shapes
- minimal diagnostic output helpers

This still does not include a lexer. The RauMa-written lexer starts in
v0.0.8c, and the self-host fixed point remains v0.0.9.

### v0.0.8c: Lexer Smoke Module

v0.0.8c adds the first `rmc` lexer module in RauMa and runs it from the
RauMa-built `rmc` binary. The current module prints a stable token smoke
sequence for a fixed demo input shape. It is not file-driven yet.

Real file-driven lexing depends on later file input support and practical
string indexing/scanning support in the bootstrap subset. The self-host fixed
point remains v0.0.9.

### v0.0.8d: Hardcoded Source Scanner

v0.0.8d adds minimal string byte support to `rmb` and uses it from `rmc` to
scan a hardcoded source string. The `rmc` lexer now loops over bytes with
`str_len` and `str_byte`, but it still does not read files or accept CLI input.

The parser still starts later, and the self-host fixed point remains v0.0.9.

### v0.0.8e: Parser Foundation

v0.0.8e adds a minimal parser foundation in RauMa. It scans the same hardcoded
demo source and recognizes only:

```text
fn main() {
    print("hi\n");
}
```

This is not a full parser and does not use a token array yet. It exists to
prove `rmc` can host parser-shaped control flow in RauMa. The self-host fixed
point remains v0.0.9.

### v0.0.8f: Token Stream Foundation

v0.0.8f separates token scanning from parser logic. `rmc/lex/lexer.rm` still
owns byte-level scanning over the hardcoded demo source, while
`rmc/lex/stream.rm` exposes cursor-style token expectations for the parser.

This still does not allocate token arrays or read files. The self-host fixed
point remains v0.0.9.

### v0.0.8g: Expanded Parser Demo

v0.0.8g parses a larger hardcoded RauMa program through token stream helpers.
The demo covers two functions, simple parameters, a return statement, a
variable declaration, and call statements.

This still does not parse arbitrary files and still has no token arrays, CLI
input, checker, or code generator in `rmc`. The self-host fixed point remains
v0.0.9.

### v0.0.8h: CLI Dispatch

v0.0.8h adds minimal generated-binary argument support to `rmb` through an
opaque `Args` type and the `args_len`, `args_get`, and `str_eq` builtins.
`rmc` uses that support for command dispatch:

- `help`
- `version`
- `demo-lex`
- `demo-parse`

The lexer and parser demos still use hardcoded source text. File input, real
compiler commands, and the self-host fixed point remain later work.

### v0.0.8i: File Input Smoke

v0.0.8i adds a temporary `read_file(path str) str` builtin to generated RauMa
programs. It reads a whole file into a string and returns an empty string on
failure. Returned buffers are intentionally leaked for now because `rmc` does
not have a proper allocator or error model yet.

`rmc demo-file <path>` uses this builtin to print file byte length and first
byte. It still does not lex or parse file contents. The self-host fixed point
remains v0.0.9.

### v0.0.8j: File-Driven Lexing

v0.0.8j adds `rmc lex <path>`. The command reads file contents with
`read_file` and feeds those bytes to the RauMa-written lexer.

This is only tokenization. `rmc` still does not parse, check, or compile file
contents, and the self-host fixed point remains v0.0.9.

### v0.0.8k: File-Driven Parse Smoke

v0.0.8k adds `rmc parse <path>`. The command reads file contents with
`read_file` and parses the same small subset handled by the parser demo through
token stream helpers.

This is still not a full parser. `rmc` does not typecheck or codegen file
contents yet, and the self-host fixed point remains v0.0.9.

### v0.0.8l: Generalized Parser Summary

v0.0.8l keeps `rmc parse <path>` file-driven but removes the single exact demo
shape. The parser now computes summaries from the input for known function
names, parameter lists, optional return types, return statements, variable
statements, and simple call statements.

AST storage is still summary-oriented, not heap-allocated. `rmc` still does not
typecheck or codegen file contents, and the self-host fixed point remains
v0.0.9.

### v0.0.8m: Token Text Handling

v0.0.8m reduces hardcoded parser names. The token stream exposes token start
and end offsets plus text printing/equality helpers, so parser summaries can
print names directly from source text.

The parser still uses cursor-style stream access and summary output. It still
does not build a heap AST, typecheck, or codegen file contents, and the
self-host fixed point remains v0.0.9.

### v0.0.8n: Lightweight Check Command

v0.0.8n adds `rmc check <path>`. The command reads a file, validates that the
supported subset parses, counts functions, and checks that functions with an
explicit return type contain a return statement.

This is not full type checking. There is no type inference, name resolution,
symbol table, HIR/MIR, or code generation in `rmc` yet. The self-host fixed
point remains v0.0.9.

### v0.0.8o: emit-c Bridge

v0.0.8o adds `rmc emit-c <path>`. The command reads a file, runs the
lightweight checker, and emits C text to stdout for tiny recognized source
shapes.

This is bridge output, not the full backend. `rmc` still does not compile or
link generated C by itself, and the self-host fixed point remains v0.0.9.

### v0.0.8p: External C Compile Workflow

v0.0.8p formalizes and tests the external compile pipeline that consumes the
output of `rmc emit-c`:

```bash
rmc emit-c input.rm > out.c
gcc -std=c11 -Wall -Wextra -Werror -pedantic out.c -o out
./out
```

The fixtures `rmb/tests/rmc_emit_workflow_hello.rm`,
`rmb/tests/rmc_emit_workflow_add.rm`, and
`rmb/tests/rmc_emit_workflow_fail.rm` exercise the bridge end-to-end. Hello
prints `hello from rmc`, add prints `42`, and the failure fixture is rejected
by the checker before any C is emitted.

This proves that the C produced by the RauMa-written `rmc` can be compiled and
executed through an external C compiler. `rmc` itself still does not invoke
`gcc`, run processes, or write files — those remain later milestones (v0.0.8q
adds an `rmc build` bridge that wraps this workflow). The self-host fixed
point remains v0.0.9.

### Stage 2: rmc2 (Second RauMa Compiler)
- Written in full RauMa
- Compiled by rmc1
- Should be identical to rmc1 in behavior
- Self-compilation test

### Stage 3: rmc3 (Third RauMa Compiler)
- Written in full RauMa  
- Compiled by rmc2
- Must produce identical output to rmc2
- Verification of self-hosting stability

## Bootstrap Verification

### Fixed Point Verification
The bootstrap is successful when:
1. `rmb` compiles `rmc1` source successfully
2. `rmc1` compiles `rmc2` source successfully
3. `rmc2` compiles `rmc3` source successfully
4. `rmc2` and `rmc3` produce identical behavior on all test inputs

### Test Suite Verification
All three compilers must pass:
- Same unit tests
- Same integration tests  
- Same regression tests
- Same performance tests (within tolerance)

## Bootstrap Language Subset

### Included Features (Stage 0)
- Basic types: `int`, `float`, `bool`, `str`
- Functions with explicit return types
- Variables with type inference
- Control flow: `if`, `while`, `for`
- Structures
- Basic error handling with `!!` and `?`
- Import/export declarations

### Excluded Features (Stage 0)
- Advanced type system (generics, traits)
- Complex error handling patterns
- Concurrency primitives
- Standard library beyond basics
- Optimization passes
- Multiple backends

## Bootstrap Implementation Details

### rmb Structure
```
rmb/
├── include/rmb/     # C headers
├── src/            # C source files
├── tests/          # Bootstrap tests
└── build/          # Build output
```

### rmb Compilation Process
1. Parse RauMa source to AST
2. Basic type checking and resolution
3. Generate C99/C11 code
4. Invoke system C compiler (gcc/clang)
5. Link into executable

### Generated C Code Characteristics
- Simple, readable C output
- No complex macros
- Minimal dependencies
- Portable across platforms
- Debuggable with standard tools

## Self-Hosting Milestones

### Milestone 1: rmb Completeness
- `rmb` can compile simple RauMa programs
- Basic test suite passes
- Ready to compile `rmc1`

### Milestone 2: rmc1 Development
- Write `rmc1` in bootstrap subset
- `rmb` successfully compiles `rmc1`
- `rmc1` passes all bootstrap tests

### Milestone 3: rmc2 Self-Compilation
- `rmc1` compiles `rmc2` (full RauMa)
- `rmc2` passes all language tests
- Ready for fixed-point verification

### Milestone 4: Fixed Point Achieved
- `rmc2` compiles `rmc3`
- `rmc2` and `rmc3` produce identical output
- Bootstrap process verified

## Challenges and Solutions

### Challenge: Circular Dependencies
**Solution**: Careful layering of compiler components. Lower-level modules don't depend on higher-level ones.

### Challenge: Bug Propagation
**Solution**: Extensive test suites at each stage. Compare outputs between stages.

### Challenge: Performance Regression
**Solution**: Benchmark each stage. Ensure no significant performance degradation.

### Challenge: Platform Differences
**Solution**: Use portable C code generation. Test on multiple platforms.

## Bootstrap Testing Strategy

### Unit Testing
- Test each compiler component in isolation
- Mock dependencies where needed
- Ensure correctness of individual pieces

### Integration Testing
- Test full compilation pipeline
- Verify generated code executes correctly
- Test error handling and diagnostics

### Comparison Testing
- Compare output of different compiler stages
- Ensure behavioral equivalence
- Detect regressions early

### Property Testing
- Generate random valid programs
- Verify all compilers handle them correctly
- Test edge cases automatically

## Bootstrap Maintenance

### Stability Guarantees
- `rmb` codebase remains stable
- Only bug fixes and essential improvements
- No new features added to `rmb`
- All evolution happens in `rmc`

### Version Compatibility
- `rmb` must always compile the bootstrap subset
- `rmc` must maintain backward compatibility with bootstrap
- Breaking changes require migration path

### Documentation
- Clear bootstrap process documentation
- Step-by-step build instructions
- Troubleshooting guide
- Platform-specific notes

## Future Bootstrap Extensions

### Cross-Compilation Support
- Bootstrap compiler can target multiple architectures
- Support for embedded systems
- WebAssembly compilation

### Alternative Bootstrap Languages
- Potential bootstrap from other languages (Go, Rust)
- Redundancy for verification
- Educational value

### Formal Verification
- Prove compiler correctness mathematically
- Certified compilation
- Eliminate entire classes of bugs
