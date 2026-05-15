// RauMa Bootstrap Compiler - Source Span Implementation
// v0.0.2: Source location and span utilities

#include "rmb/span.h"

// Create a span from start and end positions
RmbSpan rmb_span_create(const char* file, size_t start, size_t end, int line, int col) {
    RmbSpan span;
    span.file = file;
    span.start = start;
    span.end = end;
    span.line = line;
    span.col = col;
    return span;
}


// Create a zero-length span at a position
RmbSpan rmb_span_at(const char* file, size_t pos, int line, int col) {
    return rmb_span_create(file, pos, pos, line, col);
}

// Merge two spans (covers from start of first to end of second)
RmbSpan rmb_span_merge(RmbSpan a, RmbSpan b) {
    RmbSpan result;
    result.file = a.file;  // Use first span's file

    // Take minimum start and maximum end
    result.start = a.start < b.start ? a.start : b.start;
    result.end = a.end > b.end ? a.end : b.end;

    // Use line/col from first span
    result.line = a.line;
    result.col = a.col;

    return result;
}
