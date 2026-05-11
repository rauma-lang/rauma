// RauMa Bootstrap Compiler - Source Span Tracking
// v0.0.2: Source location and span utilities

#ifndef RMB_SPAN_H
#define RMB_SPAN_H

#include "common.h"
#include "source.h"
#include "string.h"

// Source location span
typedef struct RmbSpan {
    const char* file;      // Source file path (not owned)
    size_t start;          // Byte offset of span start
    size_t end;            // Byte offset after span end
    int line;              // 1-based line number
    int col;               // 1-based column number
} RmbSpan;

// Create a span from start and end positions
RmbSpan rmb_span_create(const char* file, size_t start, size_t end, int line, int col);

// Create a zero-length span at a position
RmbSpan rmb_span_at(const char* file, size_t pos, int line, int col);

// Merge two spans (covers from start of first to end of second)
RmbSpan rmb_span_merge(RmbSpan a, RmbSpan b);

// Check if span is valid (non-zero length)
static RMB_INLINE bool rmb_span_is_valid(RmbSpan span) {
    return span.start < span.end;
}

// Get the source slice for a span
static RMB_INLINE rmb_string rmb_span_slice(const RmbSource* source, RmbSpan span) {
    return rmb_source_slice(source, span.start, span.end);
}

// Unknown/empty span
#define RMB_SPAN_UNKNOWN {NULL, 0, 0, 0, 0}

#endif // RMB_SPAN_H
