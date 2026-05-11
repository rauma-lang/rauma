// RauMa Bootstrap Compiler - String Utilities
// Basic string view operations.

#ifndef RMB_STRING_H
#define RMB_STRING_H

#include "common.h"

// String view structure
typedef struct {
    const char* ptr;
    rmb_size len;
} rmb_string;

// Create string view from C string
rmb_string rmb_string_from_cstr(const char* cstr);

// Create string view from buffer
rmb_string rmb_string_from_buf(const char* buf, rmb_size len);

// Compare two strings (lexicographic)
// Returns <0 if a < b, 0 if a == b, >0 if a > b
int rmb_string_compare(rmb_string a, rmb_string b);

// Check if two strings are equal
bool rmb_string_equal(rmb_string a, rmb_string b);

// Check if string starts with prefix
bool rmb_string_starts_with(rmb_string str, rmb_string prefix);

// Check if string ends with suffix
bool rmb_string_ends_with(rmb_string str, rmb_string suffix);

// Find character in string
// Returns pointer to first occurrence, or NULL if not found
const char* rmb_string_find_char(rmb_string str, char ch);

// Find substring in string
// Returns pointer to first occurrence, or NULL if not found
const char* rmb_string_find(rmb_string str, rmb_string substr);

// Create a null-terminated copy of string
// Caller must free the result with free()
char* rmb_string_to_cstr(rmb_string str);

// Slice a string
rmb_string rmb_string_slice(rmb_string str, rmb_size start, rmb_size end);

// Trim whitespace from both ends
rmb_string rmb_string_trim(rmb_string str);

// Check if string is empty
static RMB_INLINE bool rmb_string_is_empty(rmb_string str) {
    return str.len == 0;
}

// String literals helper macro
#define RMB_STRING_LIT(s) { (s), sizeof(s) - 1 }

#endif // RMB_STRING_H
