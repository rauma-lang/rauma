// RauMa Bootstrap Compiler - String Implementation

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "rmb/string.h"

// Create string from C string
rmb_string rmb_string_from_cstr(const char* cstr) {
    rmb_string str;

    if (cstr) {
        str.ptr = cstr;
        str.len = strlen(cstr);
    } else {
        str.ptr = "";
        str.len = 0;
    }

    return str;
}


// Create string from buffer
rmb_string rmb_string_from_buf(const char* buf, rmb_size len) {
    rmb_string str;

    if (buf && len > 0) {
        str.ptr = buf;
        str.len = len;
    } else {
        str.ptr = "";
        str.len = 0;
    }

    return str;
}

// Compare two strings
int rmb_string_compare(rmb_string a, rmb_string b) {
    rmb_size min_len = a.len < b.len ? a.len : b.len;
    int result = memcmp(a.ptr, b.ptr, min_len);

    if (result != 0) {
        return result;
    }

    // Strings equal up to min_len, compare lengths
    if (a.len < b.len) {
        return -1;
    } else if (a.len > b.len) {
        return 1;
    }

    return 0;
}

// Check if two strings are equal
bool rmb_string_equal(rmb_string a, rmb_string b) {
    if (a.len != b.len) {
        return false;
    }

    return memcmp(a.ptr, b.ptr, a.len) == 0;
}

// Check if string starts with prefix
bool rmb_string_starts_with(rmb_string str, rmb_string prefix) {
    if (prefix.len > str.len) {
        return false;
    }

    return memcmp(str.ptr, prefix.ptr, prefix.len) == 0;
}

// Check if string ends with suffix
bool rmb_string_ends_with(rmb_string str, rmb_string suffix) {
    if (suffix.len > str.len) {
        return false;
    }

    const char* str_end = str.ptr + str.len - suffix.len;
    return memcmp(str_end, suffix.ptr, suffix.len) == 0;
}

// Find character in string
const char* rmb_string_find_char(rmb_string str, char ch) {
    for (rmb_size i = 0; i < str.len; i++) {
        if (str.ptr[i] == ch) {
            return str.ptr + i;
        }
    }

    return NULL;
}

// Find substring in string
const char* rmb_string_find(rmb_string str, rmb_string substr) {
    if (substr.len == 0) {
        return str.ptr;
    }

    if (substr.len > str.len) {
        return NULL;
    }

    // Simple search (could be optimized)
    rmb_size max_pos = str.len - substr.len;
    for (rmb_size i = 0; i <= max_pos; i++) {
        if (memcmp(str.ptr + i, substr.ptr, substr.len) == 0) {
            return str.ptr + i;
        }
    }

    return NULL;
}

// Create null-terminated copy
char* rmb_string_to_cstr(rmb_string str) {
    char* cstr = malloc(str.len + 1);
    if (!cstr) {
        return NULL;
    }

    memcpy(cstr, str.ptr, str.len);
    cstr[str.len] = '\0';

    return cstr;
}

// Slice a string
rmb_string rmb_string_slice(rmb_string str, rmb_size start, rmb_size end) {
    // Clamp to valid range
    if (start > str.len) {
        start = str.len;
    }

    if (end > str.len) {
        end = str.len;
    }

    if (start > end) {
        start = end;
    }

    rmb_string result;
    result.ptr = str.ptr + start;
    result.len = end - start;

    return result;
}

// Trim whitespace from both ends
rmb_string rmb_string_trim(rmb_string str) {
    const char* start = str.ptr;
    const char* end = str.ptr + str.len;

    // Trim left
    while (start < end && isspace((unsigned char)*start)) {
        start++;
    }

    // Trim right
    while (end > start && isspace((unsigned char)*(end - 1))) {
        end--;
    }

    rmb_string result;
    result.ptr = start;
    result.len = end - start;

    return result;
}
