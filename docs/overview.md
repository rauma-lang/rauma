# RauMa Language Overview

## Vision

RauMa is a compiled, no-runtime, server-side-oriented programming language designed for clarity, safety, and performance.

## Core Principles

### Simple C-like Syntax
- Familiar syntax for C, C++, Go, and Rust developers
- Curly braces for blocks, semicolons for statement termination
- Function-first design

### Type Inference by Default
- Local variables infer their types from initialization
- Function parameter types can be inferred in many cases
- Explicit types required at public/module boundaries

### Explicit Types at Important Boundaries
- Public APIs require explicit type annotations
- Module interfaces are type-checked strictly
- Internal implementation can use inference for brevity

### No Runtime
- No garbage collector
- No JIT compiler
- No virtual machine
- Direct compilation to native code

### Server-side First
- Designed for backend services, systems programming, and CLI tools
- Memory and performance conscious
- Concurrency model suitable for server workloads

### Self-hosting Goal
- The compiler must be able to compile itself
- Bootstrap compiler (rmb) is minimal and stable
- Main compiler (rmc) is written in RauMa

## Syntax Examples

### Basic Function
```rauma
fn main() {
    print("hello rauma\n");
}
```

### Type Inference
```rauma
fn add(a, b) {
    return a + b;
}

let x = 42;          // inferred as int
let name = "rauma";  // inferred as str
```

### Structures
```rauma
pub struct User {
    id int;
    name str;
    email str;
}
```

### Error Handling
```rauma
pub fn find_user(id int) User !! DbError {
    return db.find(id)?;
}
```

## Error Syntax

- `T` - Normal value type
- `T !! E` - Value `T` or error `E`
- `?` - Propagate error
- `!` - Intentional panic
- `else err { ... }` - Local error handling

## Future Directions

- Interference-based ownership model
- Minimal standard library
- Multiple backend targets (C, LLVM, custom)
- Package and module system