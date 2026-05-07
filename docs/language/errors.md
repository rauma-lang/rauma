# RauMa Error Handling

RauMa uses a simple but powerful error handling system designed for clarity and safety.

## Error Type Syntax

### `!!` - Error Annotation
```rauma
// Function that returns a value or an error
fn open_file(path str) File !! IoError {
    // Implementation
}

// Function with multiple error types
fn parse_config(data []byte) Config !! ParseError !! IoError {
    // Can return Config, ParseError, or IoError
}
```

### `?` - Error Propagation
```rauma
fn read_file_contents(path str) str !! IoError {
    let file = open_file(path)?;  // Propagates IoError if open_file fails
    return read_all(file)?;       // Propagates IoError if read_all fails
}
```

### `!` - Intentional Panic
```rauma
fn divide(a float, b float) float {
    if b == 0.0 {
        ! "division by zero";  // Intentional panic with message
    }
    return a / b;
}
```

### `else err` - Local Error Handling
```rauma
fn try_process(path str) {
    let file = open_file(path);
    else err {
        print("Failed to open file: ");
        print(err);
        return;
    }
    
    // file is guaranteed to be valid here
    process(file);
}
```

## Error Types

### Defining Error Types
```rauma
// Simple error enum
enum IoError {
    NotFound;
    PermissionDenied;
    DiskFull;
    Other(msg str);
}

// Structured error
enum ParseError {
    UnexpectedToken(token Token);
    MissingClosingBrace;
    InvalidNumber(literal str);
}
```

### Using Error Types
```rauma
fn open_file(path str) File !! IoError {
    if !file_exists(path) {
        return IoError::NotFound;
    }
    if !has_permission(path) {
        return IoError::PermissionDenied;
    }
    // ... success case returns File
}
```

## Error Handling Patterns

### Basic Propagation
```rauma
fn process_data() Result !! IoError !! ParseError {
    let raw = read_file("data.json")?;      // Propagates IoError
    let parsed = parse_json(raw)?;          // Propagates ParseError
    return transform(parsed);               // Returns Result or error
}
```

### Error Mapping
```rauma
fn load_config() Config !! ConfigError {
    let file = open_file("config.toml");
    else err {
        // Map IoError to ConfigError
        return ConfigError::IoFailed(err);
    }
    
    let config = parse_toml(file);
    else err {
        return ConfigError::ParseFailed(err);
    }
    
    return config;
}
```

### Multiple Error Recovery
```rauma
fn try_multiple_paths() File {
    // Try primary path
    let file = open_file("/etc/config.json");
    else err {
        // Try fallback
        let file = open_file("./config.json");
        else err {
            // Try default
            let file = open_file("/usr/share/default.json");
            else err {
                ! "No config file found";
            }
            return file;
        }
        return file;
    }
    return file;
}
```

### Error Inspection
```rauma
fn handle_error(result Result !! IoError !! ParseError) {
    match result {
        Result::Ok(value) => {
            print("Success: ");
            print(value);
        }
        Result::Err(IoError::NotFound) => {
            print("File not found");
        }
        Result::Err(IoError::PermissionDenied) => {
            print("Permission denied");
        }
        Result::Err(ParseError::UnexpectedToken(token)) => {
            print("Unexpected token: ");
            print(token);
        }
        Result::Err(other) => {
            print("Other error: ");
            print(other);
        }
    }
}
```

## Best Practices

### 1. Use Specific Error Types
```rauma
// Good: Specific error type
fn parse_int(s str) int !! ParseError { ... }

// Bad: Generic error
fn parse_int(s str) int !! str { ... }
```

### 2. Document Error Conditions
```rauma
/// Parse JSON data from string
///
/// Returns:
/// - Ok(Value) on success
/// - Err(ParseError::InvalidSyntax) on syntax error
/// - Err(ParseError::DepthExceeded) on nesting too deep
fn parse_json(data str) Value !! ParseError { ... }
```

### 3. Handle Errors Locally When Possible
```rauma
// Good: Handle error close to where it occurs
fn load_default_config() Config {
    let config = load_config_file();
    else err {
        return create_default_config();
    }
    return config;
}
```

### 4. Don't Ignore Errors
```rauma
// Bad: Error silently ignored
let _ = open_file("data.txt");

// Good: Error handled or propagated
let file = open_file("data.txt")?;
// or
let file = open_file("data.txt");
else err { /* handle it */ }
```

## Error Conversion

### Converting Between Error Types
```rauma
fn api_call() ApiResponse !! ApiError {
    let data = fetch_data();
    else err {
        // Convert network error to API error
        return ApiError::NetworkFailure(err);
    }
    
    let response = parse_response(data);
    else err {
        return ApiError::InvalidResponse(err);
    }
    
    return response;
}
```

### Wrapping Errors
```rauma
enum AppError {
    ConfigError(ConfigError);
    DatabaseError(DbError);
    NetworkError(NetworkError);
}

fn startup() bool !! AppError {
    let config = load_config();
    else err {
        return AppError::ConfigError(err);
    }
    
    let db = connect_db(config);
    else err {
        return AppError::DatabaseError(err);
    }
    
    return true;
}
```