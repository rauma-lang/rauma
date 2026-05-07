// RauMa Bootstrap Compiler - Diagnostics Implementation

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#else
#include <unistd.h>
#endif

#include "rmb/diag.h"

// Diagnostic context
typedef struct {
    FILE* out_stream;
    FILE* err_stream;
    bool color_enabled;
    bool warnings_enabled;
    rmb_size error_count;
    rmb_size warning_count;
} rmb_diag_context;

static rmb_diag_context g_context;

// Color codes (ANSI)
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"
#define COLOR_BOLD    "\033[1m"

// Initialize diagnostics
void rmb_diag_init(void) {
    // Reset counters
    g_context.error_count = 0;
    g_context.warning_count = 0;

    // Default streams
    g_context.out_stream = stdout;
    g_context.err_stream = stderr;

    // Enable color by default if terminal supports it
#ifdef _WIN32
    // Windows console color detection would go here
    g_context.color_enabled = false;
#else
    // Check if stderr is a terminal
    g_context.color_enabled = isatty(fileno(stderr));
#endif

    // Enable warnings by default
    g_context.warnings_enabled = true;
}

// Shutdown diagnostics
void rmb_diag_shutdown(void) {
    // Nothing to clean up for now
}

// Format message with varargs
static char* format_message(const char* format, va_list args) {
    // Determine needed size
    va_list args_copy;
    va_copy(args_copy, args);
    int needed = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);

    if (needed < 0) {
        return NULL;
    }

    // Allocate buffer
    char* buffer = malloc(needed + 1);
    if (!buffer) {
        return NULL;
    }

    // Format message
    vsnprintf(buffer, needed + 1, format, args);
    buffer[needed] = '\0';

    return buffer;
}

// Get severity string
static const char* severity_string(rmb_diag_severity severity, bool color) {
    switch (severity) {
        case RMB_DIAG_NOTE:
            return color ? COLOR_CYAN "note:" COLOR_RESET : "note:";
        case RMB_DIAG_WARNING:
            return color ? COLOR_YELLOW "warning:" COLOR_RESET : "warning:";
        case RMB_DIAG_ERROR:
            return color ? COLOR_RED "error:" COLOR_RESET : "error:";
        case RMB_DIAG_FATAL:
            return color ? COLOR_RED COLOR_BOLD "fatal error:" COLOR_RESET : "fatal error:";
        default:
            return "";
    }
}

// Print source location
static void print_location(FILE* stream, rmb_source_loc loc, bool color) {
    if (loc.filename) {
        if (color) {
            fprintf(stream, COLOR_BLUE "%s" COLOR_RESET ":", loc.filename);
        } else {
            fprintf(stream, "%s:", loc.filename);
        }
    }

    if (loc.line > 0) {
        if (color) {
            fprintf(stream, COLOR_GREEN "%zu" COLOR_RESET ":", loc.line);
        } else {
            fprintf(stream, "%zu:", loc.line);
        }

        if (loc.column > 0) {
            if (color) {
                fprintf(stream, COLOR_GREEN "%zu" COLOR_RESET ":", loc.column);
            } else {
                fprintf(stream, "%zu:", loc.column);
            }
        }
    }

    if (loc.filename || loc.line > 0) {
        fprintf(stream, " ");
    }
}

// Report diagnostic
void rmb_diag_report(rmb_diag_severity severity,
                     rmb_source_loc location,
                     const char* format, ...) {
    // Skip warnings if disabled
    if (severity == RMB_DIAG_WARNING && !g_context.warnings_enabled) {
        return;
    }

    // Update counters
    switch (severity) {
        case RMB_DIAG_ERROR:
        case RMB_DIAG_FATAL:
            g_context.error_count++;
            break;
        case RMB_DIAG_WARNING:
            g_context.warning_count++;
            break;
        default:
            break;
    }

    // Choose output stream
    FILE* stream = (severity >= RMB_DIAG_ERROR) ? g_context.err_stream : g_context.out_stream;

    // Format message
    va_list args;
    va_start(args, format);
    char* message = format_message(format, args);
    va_end(args);

    if (!message) {
        return;
    }

    // Print diagnostic
    print_location(stream, location, g_context.color_enabled);
    fprintf(stream, "%s %s\n", severity_string(severity, g_context.color_enabled), message);

    free(message);
}

// Simple error reporting
void rmb_diag_error(const char* format, ...) {
    rmb_source_loc unknown = {NULL, 0, 0, 0};
    va_list args;
    va_start(args, format);
    rmb_diag_report(RMB_DIAG_ERROR, unknown, format, args);
    va_end(args);
}

// Simple warning reporting
void rmb_diag_warning(const char* format, ...) {
    rmb_source_loc unknown = {NULL, 0, 0, 0};
    va_list args;
    va_start(args, format);
    rmb_diag_report(RMB_DIAG_WARNING, unknown, format, args);
    va_end(args);
}

// Simple info reporting
void rmb_diag_info(const char* format, ...) {
    rmb_source_loc unknown = {NULL, 0, 0, 0};
    va_list args;
    va_start(args, format);
    rmb_diag_report(RMB_DIAG_NOTE, unknown, format, args);
    va_end(args);
}

// Check if any errors
bool rmb_diag_has_errors(void) {
    return g_context.error_count > 0;
}

// Get error count
rmb_size rmb_diag_error_count(void) {
    return g_context.error_count;
}

// Get warning count
rmb_size rmb_diag_warning_count(void) {
    return g_context.warning_count;
}

// Clear diagnostics
void rmb_diag_clear(void) {
    g_context.error_count = 0;
    g_context.warning_count = 0;
}

// Set output streams
void rmb_diag_set_streams(FILE* out_stream, FILE* err_stream) {
    if (out_stream) {
        g_context.out_stream = out_stream;
    }
    if (err_stream) {
        g_context.err_stream = err_stream;
    }
}

// Enable/disable color
void rmb_diag_set_color(bool enabled) {
    g_context.color_enabled = enabled;
}

// Enable/disable warnings
void rmb_diag_set_warnings_enabled(bool enabled) {
    g_context.warnings_enabled = enabled;
}

// Span-aware error reporting
void rmb_diag_error_at(RmbSpan span, const char* format, ...) {
    // Skip if no file (empty span)
    if (!span.file) {
        return;
    }

    // Update error counter
    g_context.error_count++;

    // Format message
    va_list args;
    va_start(args, format);
    char* message = format_message(format, args);
    va_end(args);

    if (!message) {
        return;
    }

    // Print diagnostic (no color for now)
    fprintf(g_context.err_stream, "%s:%d:%d: error: %s\n",
            span.file, span.line, span.col, message);

    free(message);
}

// Span-aware warning reporting
void rmb_diag_warning_at(RmbSpan span, const char* format, ...) {
    // Skip warnings if disabled
    if (!g_context.warnings_enabled) {
        return;
    }

    // Skip if no file (empty span)
    if (!span.file) {
        return;
    }

    // Update warning counter
    g_context.warning_count++;

    // Format message
    va_list args;
    va_start(args, format);
    char* message = format_message(format, args);
    va_end(args);

    if (!message) {
        return;
    }

    // Print diagnostic (no color for now)
    fprintf(g_context.err_stream, "%s:%d:%d: warning: %s\n",
            span.file, span.line, span.col, message);

    free(message);
}

// Span-aware note reporting
void rmb_diag_note_at(RmbSpan span, const char* format, ...) {
    // Skip if no file (empty span)
    if (!span.file) {
        return;
    }

    // Format message
    va_list args;
    va_start(args, format);
    char* message = format_message(format, args);
    va_end(args);

    if (!message) {
        return;
    }

    // Print diagnostic (no color for now)
    fprintf(g_context.out_stream, "%s:%d:%d: note: %s\n",
            span.file, span.line, span.col, message);

    free(message);
}