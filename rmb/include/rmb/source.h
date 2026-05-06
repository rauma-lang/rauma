// RauMa Bootstrap Compiler - Source File Loading
// v0.0.2: Basic source file reading

#ifndef RMB_SOURCE_H
#define RMB_SOURCE_H

#include "common.h"
#include "string.h"

// Source file representation
typedef struct RmbSource {
    const char* path;      // File path (not owned)
    char* data;            // Null-terminated file content
    size_t len;            // Length in bytes (excluding null terminator)
} RmbSource;

// Read a source file into memory
// Returns true on success, false on failure
// On failure, diagnostic is printed
bool rmb_source_read(const char* path, RmbSource* out);

// Free source file memory
void rmb_source_free(RmbSource* source);

// Get a substring from source
static RMB_INLINE rmb_string rmb_source_slice(const RmbSource* source,
                                               size_t start,
                                               size_t end) {
    RMB_ASSERT(start <= end);
    RMB_ASSERT(end <= source->len);

    rmb_string str;
    str.ptr = source->data + start;
    str.len = end - start;
    return str;
}

// Get a character at position
static RMB_INLINE char rmb_source_char_at(const RmbSource* source, size_t pos) {
    if (pos >= source->len) {
        return '\0';
    }
    return source->data[pos];
}

#endif // RMB_SOURCE_H