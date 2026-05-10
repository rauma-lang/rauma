# RauMa Roadmap

## Development Stages

### v0.0.1 Repository + Bootstrap Skeleton ✓ DONE
- Initialize repository structure
- Create placeholder files
- Set up `rmb` bootstrap compiler skeleton
- Define basic language syntax and architecture

### v0.0.2 Lexer ✓ DONE
- Implement tokenizer/lexer
- Support basic RauMa syntax tokens
- Add span tracking for error reporting
- Create diagnostic system

### v0.0.3 Parser ✓ DONE
- Implement recursive descent parser
- Parse functions, variables, expressions
- Build AST structure
- Handle basic error recovery

### v0.0.4 AST + Diagnostics ✓ DONE
- Complete AST representation with spans
- Improve parser diagnostics and error recovery
- Parse match statements and enum variant payload fields
- Refine AST summary output
- Add test coverage for new syntax

### v0.0.5 Type Checking ✓ DONE
- Basic type representation (primitives, named, pointer, slice, array, optional)
- Global symbol collection (functions, structs, enums)
- Local variable scopes with simple type inference
- Function signature and return checks
- Error syntax `!!`, `?`, `!`, and `else` checking
- `rmb check <file>` command

### v0.0.6 C Codegen ✓ DONE
- Generate C code from typed AST
- Implement basic C backend (single-file `rmb build`)
- Compile simple RauMa programs to executable via `gcc`
- Test bootstrap compilation pipeline (hello, functions, control flow, structs)
- Reject error-returning functions, `match`, `defer`, `?`, and `!` with clear diagnostics

### v0.0.7 Chunk Build System ✓ DONE
- Implement file-based chunk compilation
- Track dependencies between chunks
- Parse simple local `use` declarations
- Emit per-file C, object, and `.rmi` artifacts under the fixed debug/native layout
- Link chunk objects into one executable
- Defer full incremental rebuilds and multiple compilation targets

### v0.0.8 Write `rmc` in RauMa ✓ DONE
- v0.0.8a rmc skeleton ✓ DONE
- v0.0.8b rmc source/span/token ✓ DONE
- v0.0.8c rmc lexer smoke ✓ DONE
- v0.0.8d rmc real string scanner ✓ DONE
- v0.0.8e rmc parser foundation ✓ DONE
- v0.0.8f rmc token stream ✓ DONE
- v0.0.8g rmc expanded parser demo ✓ DONE
- v0.0.8h rmc CLI dispatch ✓ DONE
- v0.0.8i rmc file input smoke ✓ DONE
- v0.0.8j rmc lex file command ✓ DONE
- v0.0.8k rmc parse file command ✓ DONE
- v0.0.8l rmc parser generalization ✓ DONE
- v0.0.8m rmc token text handling ✓ DONE
- v0.0.8n rmc check command ✓ DONE
- v0.0.8o rmc emit-c bridge ✓ DONE
- v0.0.8p rmc external C compile workflow ✓ DONE
- v0.0.8q runtime bridge primitives ✓ DONE
- v0.0.8r rmc build command bridge ✓ DONE
- v0.0.8s rmc build subset expansion ✓ DONE
- v0.0.8t rmc control-flow build subset ✓ DONE
- v0.0.8u tiny self-build target ✓ DONE
- v0.0.8v self-build CLI tool target ✓ DONE
- v0.0.8w rmc bridge consolidation ✓ DONE
- v0.0.8x self-build readiness audit ✓ DONE
- v0.0.8y rmc-mini standalone target ✓ DONE
- v0.0.8z v0.0.8 stabilization ✓ DONE
- Complete the `rmc` bridge milestone: `rmb` builds `rmc`, and `rmc` builds
  supported single-file targets including tiny/tool/rmc-mini.
- Defer real self-host fixed point, multi-file `rmc build`, HIR/MIR, and full
  backend work to v0.0.9 and later.

### v0.0.9 Self-host Fixed Point
- v0.0.9a standalone rmc-mini hardening ✓ DONE
- v0.0.9b rmc multi-file build foundation ✓ DONE
- v0.0.9c real rmc module group build ✓ DONE
- v0.0.9d larger rmc CLI module group build ✓ DONE
- v0.0.9e real-ish rmc frontend module group build ✓ DONE
- v0.0.9f rmc compiler candidate ✓ DONE
- v0.0.9g fixed-point candidate ✓ DONE
- v0.0.9g-fix controlled candidate chain ✓ DONE
- v0.0.9g-fix reached `rmc0 -> rmc1 -> rmc2 -> rmc3` for the controlled
  `rmb/tests/rmc_candidate/` source only.
- Do not claim real compiler self-hosting until the staged build applies to the
  real `rmc` source and comparison strategy.
- Next options: v0.1.0 stabilization planning, or v0.0.9h real-rmc self-host
  expansion.

### v0.1.0 Usable Compiler
- Stable self-hosting compiler
- Comprehensive test suite
- Documentation
- Example programs
- Ready for community use

## Future Considerations

### Post v1.0
- Standard library expansion
- Additional backend targets (LLVM)
- Package management system
- Language server protocol (LSP)
- Performance optimizations
- Advanced type system features
- Concurrency primitives

## Versioning Policy

- **v0.0.x** - Development phase, breaking changes expected
- **v0.1.0** - First stable release
- **Semantic Versioning** post v1.0

## Stability Goals

- `rmb` (bootstrap compiler) remains stable C11 codebase
- `rmc` (main compiler) evolves with language
- Backward compatibility prioritized post v1.0
- Clear migration paths for breaking changes
