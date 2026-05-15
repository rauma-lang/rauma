// RauMa Bootstrap Compiler - Source File Loading Implementation
// v0.0.2: Basic source file reading

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rmb/source.h"
#include "rmb/diag.h"

// Read a source file into memory
bool rmb_source_read(const char* path, RmbSource* out) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        rmb_diag_error("cannot open file: %s", path);
        return false;
    }

    // Get file size
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        rmb_diag_error("cannot seek in file: %s", path);
        return false;
    }

    long file_size = ftell(file);
    if (file_size < 0) {
        fclose(file);
        rmb_diag_error("cannot get file size: %s", path);
        return false;
    }

    // Reset to beginning
    rewind(file);

    // Allocate buffer (+1 for null terminator)
    size_t size = (size_t)file_size;
    char* buffer = malloc(size + 1);
    if (!buffer) {
        fclose(file);
        rmb_diag_error("out of memory reading file: %s", path);
        return false;
    }

    // Read file
    size_t bytes_read = fread(buffer, 1, size, file);
    if (bytes_read != size) {
        free(buffer);
        fclose(file);
        rmb_diag_error("cannot read file: %s", path);
        return false;
    }

    fclose(file);

    // Null-terminate
    buffer[size] = '\0';

    // Fill output structure
    out->path = path;
    out->data = buffer;
    out->len = size;

    return true;
}


// Free source file memory
void rmb_source_free(RmbSource* source) {
    if (source && source->data) {
        free(source->data);
        source->data = NULL;
        source->len = 0;
    }
}
