# RauMa Standard Library

## Overview

The RauMa Standard Library (`std`) provides essential functionality for RauMa programs. It is intentionally minimal, focusing on core functionality needed for systems programming and server-side applications.

## Design Principles

### 1. Minimalism
- Include only essential functionality
- Avoid feature bloat
- Keep dependencies simple

### 2. Safety
- Memory-safe by design
- Error handling built-in
- No undefined behavior

### 3. Performance
- Zero-cost abstractions where possible
- Minimal runtime overhead
- Efficient algorithms

### 4. Portability
- Work across platforms
- No platform-specific code in core
- Clear abstraction layers

## Module Structure

### Core Modules
```
std/
├── core/     # Core language support
├── fs/       # File system operations
├── mem/      # Memory management
├── os/       # OS abstractions
└── io/       # Input/output
```

### Planned Modules
- `std/net` - Networking
- `std/time` - Time and date
- `std/math` - Mathematical functions
- `std/rand` - Random number generation
- `std/json` - JSON parsing/serialization
- `std/test` - Testing framework

## Module Details

### `std/core`
Core language utilities and types.

**Features:**
- Basic types (`Option`, `Result`, `Vec`, `String`)
- Type conversion utilities
- Debug and display traits
- Comparison traits
- Hash implementation

**Example:**
```rauma
import std.core;

fn process(value Option<int>) {
    match value {
        Option::Some(x) => print("Value: {x}"),
        Option::None => print("No value"),
    }
}
```

### `std/fs`
File system operations.

**Features:**
- File reading/writing
- Directory operations
- Path manipulation
- File metadata

**Example:**
```rauma
import std.fs;

fn read_config() str !! IoError {
    let content = fs.read_file("config.json")?;
    return content;
}
```

### `std/mem`
Memory management utilities.

**Features:**
- Arena allocator
- Pool allocator  
- Memory tracking
- Alignment utilities

**Example:**
```rauma
import std.mem;

fn process_data() {
    let arena = Arena::new();
    let data = arena.alloc(1024);
    // Use allocated memory
    // Automatically freed when arena goes out of scope
}
```

### `std/os`
Operating system abstractions.

**Features:**
- Process management
- Environment variables
- Signal handling
- System information

**Example:**
```rauma
import std.os;

fn get_user() str {
    return os.getenv("USER") else "unknown";
}
```

### `std/io`
Input/output operations.

**Features:**
- Standard streams (stdin, stdout, stderr)
- Buffered I/O
- Formatting
- Serialization/deserialization

**Example:**
```rauma
import std.io;

fn interact() {
    print("Enter your name: ");
    let name = io.read_line() else "anonymous";
    print("Hello, {name}!");
}
```

## Error Handling

### Standard Error Types
```rauma
// In std/core/error.rm
enum IoError {
    NotFound;
    PermissionDenied;
    AlreadyExists;
    InvalidData;
    Other(msg str);
}

enum ParseError {
    InvalidSyntax;
    UnexpectedEOF;
    InvalidNumber;
    Other(msg str);
}
```

### Error Conversion
```rauma
import std.core;

fn try_read() Result<str> !! IoError {
    let file = fs.open("data.txt")?;
    return io.read_all(file);
}
```

## Memory Management

### Smart Pointers
```rauma
// Reference-counted pointer
let rc = Rc::new(42);
let another = rc.clone();  // Reference count increased

// Atomic reference counting (thread-safe)
let arc = Arc::new(42);

// Unique ownership pointer
let unique = Box::new(42);
```

### Custom Allocators
```rauma
import std.mem;

struct CustomAllocator {
    arena: Arena;
}

impl Allocator for CustomAllocator {
    fn alloc(self, size usize) *void {
        return self.arena.alloc(size);
    }
}
```

## Concurrency

### Basic Concurrency (Future)
```rauma
// Planned for future versions
import std.thread;

fn parallel_compute() {
    let handle = thread.spawn(|| {
        compute_heavy_task();
    });
    
    let result = handle.join();
}
```

## Testing

### Standard Test Framework
```rauma
import std.test;

test "addition works" {
    assert_eq(2 + 2, 4);
}

test "error handling" {
    let result = fallible_function();
    assert_err(result);
}
```

## Versioning

### Stability Guarantees
- Core API stable after 1.0
- Breaking changes require major version
- Deprecated features with migration path
- Long-term support for stable APIs

### Compatibility
- Backward compatibility prioritized
- Clear upgrade paths
- Migration tools where needed

## Contributing

### Adding New Functionality
1. Prove need for standard library inclusion
2. Design minimal, safe API
3. Implement with comprehensive tests
4. Document thoroughly
5. Review and iterate

### Extension Points
- Custom allocators
- Trait implementations
- Type conversions
- Error type extensions

## Future Development

### Planned Features
- **Network Stack**: TCP/UDP, HTTP client/server
- **Cryptography**: Hashing, encryption, signatures
- **Compression**: gzip, deflate, brotli
- **Serialization**: JSON, YAML, TOML, MessagePack
- **Database**: SQLite client, connection pooling

### Performance Optimization
- SIMD vector operations
- Zero-copy I/O
- Lock-free data structures
- Memory pooling

### Platform Support
- Linux, macOS, Windows
- WebAssembly
- Embedded systems
- Mobile platforms