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

### v0.0.6 C Codegen
- Generate C code from typed AST
- Implement basic C backend
- Compile simple RauMa programs to executable
- Test bootstrap compilation pipeline

### v0.0.7 Chunk Build System
- Implement file-based chunk compilation
- Track dependencies between chunks
- Incremental rebuilds
- Support multiple compilation targets

### v0.0.8 Write `rmc` in RauMa
- Port compiler from C to RauMa
- Implement self-hosting compiler core
- Test compiler bootstrapping
- Ensure feature parity with `rmb`

### v0.0.9 Self-host Fixed Point
- Achieve self-hosting compilation
- `rmb` builds `rmc1`
- `rmc1` builds `rmc2`
- `rmc2` builds `rmc3`
- Verify `rmc2` and `rmc3` produce identical output

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