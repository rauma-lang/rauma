# RauMa Type System

## Type Inference by Default

RauMa uses type inference for local variables and many function parameters, reducing boilerplate while maintaining type safety.

### Local Inference
```rauma
let x = 42;          // inferred as int
let y = 3.14;        // inferred as float
let name = "rauma";  // inferred as str
let flag = true;     // inferred as bool
```

### Function Parameter Inference
```rauma
fn add(a, b) {      // types inferred from usage
    return a + b;   // if used with ints, a and b are int
}

let result = add(10, 20);  // a and b inferred as int
```

## Explicit Types at Boundaries

While inference is convenient internally, explicit types are required at important boundaries for clarity and safety.

### Public API Boundaries
```rauma
// Public function requires explicit return type
pub fn calculate_distance(x1, x2) float {
    return abs(x2 - x1);
}

// Public struct fields require explicit types
pub struct User {
    id int;
    name str;
    email str;
}
```

### Module Interfaces
```rauma
// Module exports require explicit types
pub const MAX_USERS: int = 1000;
pub type UserId = int;

pub fn create_user(name str) User !! DbError;
```

## Basic Types

### Primitive Types
- `int` - Signed integer (platform-dependent size)
- `uint` - Unsigned integer (platform-dependent size)
- `float` - Floating point number (double precision)
- `byte` - 8-bit unsigned integer (0-255)
- `bool` - Boolean (`true` or `false`)
- `str` - String (UTF-8 encoded)

### Sized Integer Types
- `i8`, `i16`, `i32`, `i64` - Signed integers with specific sizes
- `u8`, `u16`, `u32`, `u64` - Unsigned integers with specific sizes

### Composite Types
```rauma
// Array (fixed size)
let arr: [10]int;        // array of 10 integers
let matrix: [3][3]float; // 3x3 matrix

// Slice (dynamic view into array)
let slice: []int;        // slice of integers
let str_slice: []byte;   // byte slice (for binary data)

// Pointer
let ptr: *int;           // pointer to integer
let const_ptr: *const int; // pointer to constant integer

// Reference
let ref: &int;           // reference to integer (non-nullable)

// Function pointer
let fn_ptr: fn(int, int) int;
```

### Optional Type
```rauma
let maybe_int: int? = null;      // optional integer
let maybe_str: str? = "hello";   // optional string

// Unwrapping
if let value = maybe_int {
    // value is available here
} else {
    // handle missing value
}
```

### Error Type
```rauma
// Function that returns value or error
fn open_file(path str) File !! IoError;

// Usage with error handling
let file = open_file("data.txt")?;
```

## Type Aliases
```rauma
type UserId = int;
type FilePath = str;
type Callback = fn(int, int) int;

// Generic type alias (future)
type Result<T, E> = T !! E;
```

## Type Conversion

### Explicit Conversion
```rauma
let x: int = 42;
let y: float = float(x);  // explicit conversion

let a: float = 3.14;
let b: int = int(a);      // truncates to integer
```

### Implicit Conversion
```rauma
// Widening conversions are implicit
let small: i32 = 100;
let large: i64 = small;  // implicit conversion i32 → i64

let int_val: int = 42;
let float_val: float = int_val;  // implicit int → float
```

## Type Checking Rules

1. **Inference**: Types are inferred from initialization and usage
2. **Compatibility**: Operations require compatible types
3. **Conversion**: Implicit conversion only for widening/non-lossy
4. **Boundaries**: Public APIs require explicit types
5. **Safety**: Type errors are caught at compile time

## Future Type System Features

### Generics (Planned)
```rauma
fn identity<T>(value T) T {
    return value;
}

struct Container<T> {
    value T;
}
```

### Traits/Interfaces (Planned)
```rauma
trait Display {
    fn to_string(self) str;
}

impl Display for Point {
    fn to_string(self) str {
        return format("({}, {})", self.x, self.y);
    }
}
```

### Sum Types (Planned)
```rauma
type Shape = Circle | Rectangle | Triangle;

match shape {
    Circle(radius) => { ... }
    Rectangle(width, height) => { ... }
    Triangle(a, b, c) => { ... }
}
```