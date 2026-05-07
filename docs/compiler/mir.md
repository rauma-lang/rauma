# Middle-Level IR (MIR)

## Overview

MIR (Middle-Level IR) is RauMa's platform-independent intermediate representation. It serves as the stable interface between the compiler frontend and backend, enabling multiple target code generation.

## Design Goals

### 1. Platform Independence
- No architecture-specific assumptions
- Abstract memory model
- Portable type system

### 2. Optimization Friendly
- SSA (Static Single Assignment) form
- Explicit control flow
- Easy analysis and transformation

### 3. Backend Agnostic
- Can target C, LLVM, custom backends
- Clear lowering semantics
- Well-defined operational semantics

### 4. Debuggable
- Source location preservation
- Human-readable format
- Tooling support

## MIR Structure

### Basic Blocks
```rauma
// MIR basic block
block %entry(param0: i32, param1: i32) -> i32 {
    %0 = add i32 param0, param1;
    ret %0;
}
```

### Functions
```rauma
// MIR function
function @add(%a: i32, %b: i32) -> i32 {
    // Parameters become block arguments
    block %entry(%x: i32, %y: i32) -> i32 {
        %sum = add i32 %x, %y;
        ret %sum;
    }
}
```

### Modules
```rauma
// MIR module
module @example {
    // Function declarations
    declare function @external() -> i32;
    
    // Function definitions  
    function @main() -> i32 {
        block %entry() -> i32 {
            %result = call @external();
            ret %result;
        }
    }
}
```

## MIR Types

### Primitive Types
- `i1`, `i8`, `i16`, `i32`, `i64` - Integer types
- `f32`, `f64` - Floating point types
- `ptr` - Pointer type (opaque)
- `void` - No value

### Composite Types
- `[N x T]` - Fixed array
- `[] T` - Slice (pointer + length)
- `{T1, T2, ...}` - Structure
- `(T1, T2, ...)` - Tuple

### Function Types
- `(T1, T2, ...) -> R` - Function type

## MIR Instructions

### Arithmetic Operations
```rauma
%1 = add i32 %a, %b;      // Addition
%2 = sub i32 %a, %b;      // Subtraction  
%3 = mul i32 %a, %b;      // Multiplication
%4 = div i32 %a, %b;      // Division (signed)
%5 = rem i32 %a, %b;      // Remainder
%6 = udiv i32 %a, %b;     // Division (unsigned)
%7 = urem i32 %a, %b;     // Remainder (unsigned)
```

### Bitwise Operations
```rauma
%8 = and i32 %a, %b;      // Bitwise AND
%9 = or i32 %a, %b;       // Bitwise OR
%10 = xor i32 %a, %b;     // Bitwise XOR
%11 = shl i32 %a, %b;     // Shift left
%12 = shr i32 %a, %b;     // Shift right (signed)
%13 = ushr i32 %a, %b;    // Shift right (unsigned)
```

### Memory Operations
```rauma
%14 = alloca i32;                // Stack allocation
%15 = load i32* %ptr;            // Load from pointer
store i32 %value, i32* %ptr;     // Store to pointer
%16 = gep ptr %base, i64 %idx;   // Get element pointer
```

### Control Flow
```rauma
br %cond, %true_block, %false_block;  // Conditional branch
br %target_block;                     // Unconditional branch
%17 = phi i32 [%a, %block_a], [%b, %block_b];  // Phi node
```

### Function Calls
```rauma
%18 = call i32 @func(%arg1, %arg2);    // Function call
%19 = tail call i32 @func(%arg1);      // Tail call
```

### Conversion Operations
```rauma
%20 = trunc i64 %a to i32;      // Truncate
%21 = zext i32 %a to i64;       // Zero extend
%22 = sext i32 %a to i64;       // Sign extend
%23 = fptrunc f64 %a to f32;    // Float truncate
%24 = fpext f32 %a to f64;      // Float extend
%25 = fptosi f64 %a to i32;     // Float to signed int
%26 = sitofp i32 %a to f64;     // Signed int to float
```

### Comparison Operations
```rauma
%27 = icmp eq i32 %a, %b;      // Integer equal
%28 = icmp ne i32 %a, %b;      // Integer not equal
%29 = icmp slt i32 %a, %b;     // Signed less than
%30 = icmp sgt i32 %a, %b;     // Signed greater than
%31 = fcmp olt f32 %a, %b;     // Ordered float less than
%32 = fcmp ogt f32 %a, %b;     // Ordered float greater than
```

## MIR Optimization Passes

### Local Optimizations
- **Constant folding**: Evaluate constant expressions
- **Algebraic simplification**: Simplify arithmetic
- **Dead code elimination**: Remove unused instructions
- **Common subexpression elimination**: Reuse computed values

### Control Flow Optimizations
- **Branch folding**: Eliminate unnecessary branches
- **Loop invariant code motion**: Move computations out of loops
- **Unreachable code elimination**: Remove dead blocks

### Memory Optimizations
- **Load/store elimination**: Remove redundant memory operations
- **Stack allocation promotion**: Convert heap to stack where safe
- **Memory coalescing**: Combine adjacent memory operations

## Lowering to Backends

### C Backend Lowering
```rauma
// MIR
%sum = add i32 %a, %b;

// Lowered to C
int sum = a + b;
```

### LLVM Backend Lowering
```rauma
// MIR
%sum = add i32 %a, %b;

// Lowered to LLVM IR
%sum = add i32 %a, %b  ; Same representation
```

### Custom Backend Lowering
```rauma
// MIR
%sum = add i32 %a, %b;

// Lowered to machine code
ADD R1, R2, R3  ; Example assembly
```

## MIR Serialization

### Text Format
```rauma
// Human-readable MIR
module @example version=1 {
    function @add(%a: i32, %b: i32) -> i32 {
        block %entry(%x: i32, %y: i32) -> i32 {
            %0 = add i32 %x, %y;
            ret %0;
        }
    }
}
```

### Binary Format
- Compact binary representation
- Faster loading/saving
- Suitable for caching

### JSON Format
- Machine-readable
- Tool integration
- Debugging and analysis

## MIR Validation

### Structural Validation
- SSA property verification
- Type consistency checking
- Control flow validity
- Dominator tree verification

### Semantic Validation
- Memory safety checks
- Division by zero detection
- Signed overflow checking
- Alignment verification

### Optimization Validation
- Preservation of semantics
- No observable behavior change
- Performance improvement verification

## MIR Tooling

### MIR Printer
- Pretty-print MIR
- Syntax highlighting
- Graph visualization

### MIR Diff
- Compare MIR versions
- Show optimization effects
- Debug transformation issues

### MIR Interpreter
- Execute MIR directly
- Debug compiler bugs
- Test optimization passes

## Future Extensions

### Parallel MIR
- Explicit parallelism annotations
- Data race detection
- Vector operations

### Specialized MIR
- GPU/accelerator operations
- SIMD vector types
- Atomic operations

### Verification MIR
- Formal semantics
- Proof carrying code
- Security guarantees