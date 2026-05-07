# RauMa Syntax

## Basic Syntax Elements

### Functions
```rauma
fn main() {
    print("hello rauma\n");
}

fn add(a, b) {
    return a + b;
}

pub fn public_function() int {
    return 42;
}
```

### Variables
```rauma
let x = 42;           // inferred type
let y: int = 100;     // explicit type
const PI = 3.14159;   // constant
```

### Control Flow
```rauma
// if-else
if x > 0 {
    print("positive\n");
} else if x < 0 {
    print("negative\n");
} else {
    print("zero\n");
}

// while loop
let i = 0;
while i < 10 {
    print(i);
    i = i + 1;
}

// for loop
for let i = 0; i < 10; i = i + 1 {
    print(i);
}

// for-in loop (arrays/slices)
let arr = [1, 2, 3];
for item in arr {
    print(item);
}
```

### Structures
```rauma
struct Point {
    x float;
    y float;
}

pub struct User {
    id int;
    name str;
    email str;
}

// Usage
let p = Point { x: 1.0, y: 2.0 };
let u = User { id: 1, name: "Alice", email: "alice@example.com" };
```

### Enumerations
```rauma
enum Result {
    Ok(value int);
    Err(error str);
}

enum Option {
    Some(value T);
    None;
}

// Usage
let result = Result::Ok(42);
let option = Option::Some("hello");
```

### Pattern Matching
```rauma
match (result) {
    case Ok(value) {
        print("success: ");
        print(value);
    }

    case Err(error) {
        print("error: ");
        print(error);
    }
}
```

### Error Handling
```rauma
// Function that can fail
fn open_file(path str) File !! IoError {
    // ...
}

// Error propagation
fn read_config() Config !! ConfigError {
    let file = open_file("config.json")?;
    return parse_config(file);
}

// Error handling
fn try_open() {
    let result = open_file("data.txt");
    else err {
        print("failed to open file: ");
        print(err);
        return;
    }
    // use result here
}
```

### Type Declarations
```rauma
// Basic types
let a: int = 42;
let b: uint = 100;
let c: float = 3.14;
let d: bool = true;
let e: byte = 0xFF;
let f: str = "hello";

// Pointer types
let ptr: *int = &x;

// Array types
let arr1: [10]int;      // fixed array
let arr2: []int;        // slice/dynamic array

// Optional type
let opt: int? = null;

// Error type
let result: int !! IoError = open_file_size("test.txt");
```

### Comments
```rauma
// Single line comment

/*
Multi-line
comment
*/

/// Documentation comment for functions
/// Returns the sum of a and b
fn add(a, b) {
    return a + b;
}
```

### Import/Export
```rauma
// Import module
import std.io;
import math { sin, cos };

// Export declarations
pub fn public_function() { ... }
pub struct PublicStruct { ... }
pub const PUBLIC_CONST = 42;
```

### Expression Syntax
```rauma
// Arithmetic
let sum = a + b;
let diff = a - b;
let prod = a * b;
let quot = a / b;
let rem = a % b;

// Comparison
let eq = a == b;
let ne = a != b;
let lt = a < b;
let le = a <= b;
let gt = a > b;
let ge = a >= b;

// Logical
let and = a && b;
let or = a || b;
let not = !a;

// Bitwise
let bit_and = a & b;
let bit_or = a | b;
let bit_xor = a ^ b;
let shift_left = a << 2;
let shift_right = a >> 2;

// Assignment
x = 42;
x += 1;
x -= 1;
x *= 2;
x /= 2;
x %= 3;
x &= 0xFF;
x |= 0x01;
x ^= 0xFF;
x <<= 1;
x >>= 1;
```