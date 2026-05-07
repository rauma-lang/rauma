# Interference Ownership

## Overview

Interference Ownership is RauMa's approach to memory safety and concurrency control. Instead of requiring explicit ownership annotations (like Rust's `borrow checker`), the compiler infers read/write/move interference between operations and prevents unsafe patterns at compile time.

## Core Concepts

### Interference Types
The compiler tracks how values are used and detects potential conflicts:

1. **Read Interference** - Multiple readers accessing the same data
2. **Write Interference** - Multiple writers modifying the same data  
3. **Move Interference** - Value moved while still referenced
4. **Share Interference** - Shared mutable access

### Inference vs Annotation
- Developer writes normal code without ownership annotations
- Compiler infers interference patterns from usage
- Errors reported when unsafe patterns detected
- Simple cases work automatically; complex cases may need hints

## Basic Examples

### Safe Parallel Reads
```rauma
let data = [1, 2, 3, 4, 5];

// Multiple reads are safe
let sum1 = compute_sum(data);
let sum2 = compute_average(data);
```

### Preventing Data Races
```rauma
let counter = 0;

fn increment(counter) {
    counter = counter + 1;  // Write
}

// Compiler error: write interference
let task1 = spawn { increment(counter) };
let task2 = spawn { increment(counter) };
```

### Move Semantics
```rauma
struct Buffer {
    data: []byte;
}

fn process(buf Buffer) {
    // Consumes buffer
}

let buf = Buffer { data: load_data() };
process(buf);  // buf is moved here

// Compiler error: use after move
print(buf.data);
```

## Interference Operations

### Read Operations
- Reading a value
- Copying a value (if copyable)
- Passing by immutable reference
- Returning by value

### Write Operations  
- Assigning to a variable
- Modifying through a reference
- Mutating collection elements

### Move Operations
- Passing ownership to function
- Returning ownership from function
- Reassigning ownership

## Compiler Inference

### Tracking Variable States
The compiler tracks each variable's state:
- **Available** - Can be read, written, or moved
- **Borrowed** - Currently referenced (read-only or mutable)
- **Moved** - Ownership transferred
- **Frozen** - Cannot be mutated while borrowed

### Inference Rules
1. Multiple simultaneous readers allowed
2. Single writer allowed (no other readers/writers)
3. Move transfers ownership (invalidate all references)
4. Mutable borrow excludes all other access
5. Immutable borrow excludes mutable access

## Error Messages

### Write Interference Error
```
error: write interference detected
  --> src/main.rm:10:5
   |
10 |     modify(data);
   |     ^^^^^^ cannot write to `data` while borrowed
   |
note: `data` is mutably borrowed here
  --> src/main.rm:8:15
   |
8  |     let ref = &mut data;
   |               ^^^^^^^^^ mutable borrow occurs here
```

### Move Interference Error
```
error: use after move
  --> src/main.rm:15:10
   |
12 |     let new_owner = take_ownership(value);
   |                     --------------------- `value` moved here
...
15 |     print(value);
   |           ^^^^^ value used after move
```

## Advanced Patterns

### Interior Mutability
```rauma
// Compiler allows controlled interior mutation
struct Cache {
    data: []byte;
    hits: Cell<int>;  // Interior mutable counter
}

let cache = Cache { data: load_data(), hits: Cell::new(0) };

// Safe: Cell provides synchronization
cache.hits.increment();
cache.hits.increment();
```

### Scoped Borrows
```rauma
{
    let mut_ref = &mut data;
    modify(mut_ref);
}  // Borrow ends here

// Now data can be used again
process(data);
```

### Atomic Operations
```rauma
use std.sync.Atomic;

let counter = Atomic::new(0);

// Atomic operations are interference-safe
spawn { counter.fetch_add(1); }
spawn { counter.fetch_add(1); }
```

## Comparison with Other Systems

### vs Rust Borrow Checker
- **RauMa**: Inference-based, fewer annotations
- **Rust**: Annotation-based, more explicit control
- **Trade-off**: Simplicity vs fine-grained control

### vs GC Languages
- **RauMa**: Compile-time safety, no runtime overhead
- **GC**: Runtime safety, automatic memory management  
- **Trade-off**: Performance vs development speed

### vs C/C++
- **RauMa**: Automatic safety checking
- **C/C++**: Manual memory management
- **Trade-off**: Safety vs control

## Future Directions

### Inference Improvements
- Better heuristics for complex patterns
- Reduced false positives
- More helpful error messages

### Annotation System
- Optional annotations for complex cases
- Performance hints for hot paths
- Explicit ownership transfer markers

### Concurrency Primitives
- Built-in actor model support
- Software transactional memory
- Data race detection enhancements

## Bootstrap Implementation

For the bootstrap compiler (`rmb`), only minimal interference checks will be implemented:
- Basic move detection
- Simple borrow tracking
- Essential safety guarantees

Full interference system will be implemented in the main compiler (`rmc`) written in RauMa itself.