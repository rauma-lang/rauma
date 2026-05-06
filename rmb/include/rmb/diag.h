// RauMa Bootstrap Compiler - Diagnostics
// Simple diagnostic reporting system.

#ifndef RMB_DIAG_H
#define RMB_DIAG_H

#include <stdio.h>
#include "common.h"
#include "span.h"

// Diagnostic severity levels
typedef enum {
    RMB_DIAG_NOTE,     // Informational note
    RMB_DIAG_WARNING,  // Warning (may be an issue)
    RMB_DIAG_ERROR,    // Error (prevents compilation)
    RMB_DIAG_FATAL     // Fatal error (cannot continue)
} rmb_diag_severity;

// Source location (legacy, use RmbSpan for new code)
typedef struct {
    const char* filename;   // Source file name
    rmb_size line;          // 1-based line number
    rmb_size column;        // 1-based column number
    rmb_size offset;        // Byte offset in file
} rmb_source_loc;

// Diagnostic message
typedef struct {
    rmb_diag_severity severity;
    rmb_source_loc location;
    const char* message;
} rmb_diag;

// Initialize diagnostics system
void rmb_diag_init(void);

// Shutdown diagnostics system
void rmb_diag_shutdown(void);

// Report a diagnostic (legacy API)
void rmb_diag_report(rmb_diag_severity severity,
                     rmb_source_loc location,
                     const char* format, ...);

// Simple diagnostic reporting (no location)
void rmb_diag_error(const char* format, ...);
void rmb_diag_warning(const char* format, ...);
void rmb_diag_info(const char* format, ...);

// Span-aware diagnostic reporting (new API)
void rmb_diag_error_at(RmbSpan span, const char* format, ...);
void rmb_diag_warning_at(RmbSpan span, const char* format, ...);
void rmb_diag_note_at(RmbSpan span, const char* format, ...);

// Check if any errors have been reported
bool rmb_diag_has_errors(void);

// Get number of errors reported
rmb_size rmb_diag_error_count(void);

// Get number of warnings reported
rmb_size rmb_diag_warning_count(void);

// Clear all diagnostics
void rmb_diag_clear(void);

// Set output streams
void rmb_diag_set_streams(FILE* out_stream, FILE* err_stream);

// Enable/disable color output
void rmb_diag_set_color(bool enabled);

// Enable/disable warnings
void rmb_diag_set_warnings_enabled(bool enabled);

// Create a source location (legacy)
static RMB_INLINE rmb_source_loc rmb_source_loc_create(
    const char* filename,
    rmb_size line,
    rmb_size column,
    rmb_size offset) {
    rmb_source_loc loc = {filename, line, column, offset};
    return loc;
}

// Unknown source location
#define RMB_SOURCE_LOC_UNKNOWN \
    {NULL, 0, 0, 0}

// Diagnostic reporting macros (legacy)
#define RMB_DIAG_ERROR_AT(loc, ...) \
    rmb_diag_report(RMB_DIAG_ERROR, (loc), __VA_ARGS__)

#define RMB_DIAG_WARNING_AT(loc, ...) \
    rmb_diag_report(RMB_DIAG_WARNING, (loc), __VA_ARGS__)

#define RMB_DIAG_NOTE_AT(loc, ...) \
    rmb_diag_report(RMB_DIAG_NOTE, (loc), __VA_ARGS__)

#define RMB_DIAG_ERROR(...) \
    rmb_diag_report(RMB_DIAG_ERROR, RMB_SOURCE_LOC_UNKNOWN, __VA_ARGS__)

#define RMB_DIAG_WARNING(...) \
    rmb_diag_report(RMB_DIAG_WARNING, RMB_SOURCE_LOC_UNKNOWN, __VA_ARGS__)

#define RMB_DIAG_NOTE(...) \
    rmb_diag_report(RMB_DIAG_NOTE, RMB_SOURCE_LOC_UNKNOWN, __VA_ARGS__)

#endif // RMB_DIAG_H