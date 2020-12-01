//===--------------------------------------------------------------------------------------------===
// filesystem.c - thin file system abstraction layer
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <ccore/filesystem.h>
#include <ccore/log.h>
#include <ccore/memory.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h> // TODO: remove reliance on MingW, use windows.h API?
#include <errno.h>
#include <stdint.h>

static inline bool is_path_sep(char c) {
    return c == '\\' || c == '/';
}

static size_t path_component_length(const char *comp) {
    size_t length = strlen(comp);
    while(length && is_path_sep(comp[length-1])) {
        length -= 1;
    }
    return length;
}

static char *pathcpy(char *dest, const char *src, size_t n) {
    CCASSERT(dest);
    CCASSERT(src);

    char *ret = dest;
    while(n && *src) {
        if(is_path_sep(*src)) {
            *dest = CCFS_PATH_SEP;
        } else {
            *dest = *src;
        }
        dest += 1;
        src += 1;
        n -= 1;
    }

    while (n--)
        *dest++ = 0;
    return ret;
}

size_t ccfs_path_concat(char *out, size_t size, ...) {
    CCASSERT(out);
    size_t head = 0;

    va_list components;
    va_start(components, size);

    const char *comp = NULL;
    bool is_first = true;
    while((comp = va_arg(components, const char*))) {
        size_t comp_length = path_component_length(comp);
        if(head + comp_length + 1 >= size) break;

        if(is_first) {
            is_first = false;
        } else {
            out[head++] = CCFS_PATH_SEP;
        }
        pathcpy(out + head, comp, comp_length);
        head += comp_length;
    }
    va_end(components);
    out[head] = '\0';
    return head;
}

bool ccfs_path_exists(const char *path) {
    return access(path, F_OK) == 0;
}

void ccfs_path_rtrim_i(char *path, int num_dirs) {
    int length = strlen(path);
    char *head = path + length-1;

    for(int dirs = 0; dirs < num_dirs+1 && head != path; --head) {
        char c = *head;
        if(!is_path_sep(c)) continue;
        dirs += 1;
    }
    *(head+2) = '\0';
}

static const char *mode_string(ccfs_mode_t mode) {
    switch(mode) {
        case CCFS_READ: return "rb";
        case CCFS_WRITE: return "wb";
        case CCFS_APPEND: return "ab";
    }
}

FILE* ccfs_file_open(const char *path, ccfs_mode_t mode) {
    FILE* io = fopen(path, mode_string(mode));
    if(!io) {
        CCERROR("error opening `%s`: %s", path, strerror(errno));
    }
    return io;
}

void *ccfs_file_read(FILE* file, size_t *size) {
    CCASSERT(file);
    CCASSERT(size);
    *size = 0;
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);
    uint8_t *data = cc_alloc(*size + 1); // For text files, we always write a null byte at the end
    fread(data, 1, *size, file);
    data[*size] = 0x00;
    return data;
}

void *ccfs_path_read(const char *path, size_t *size) {
    FILE *f = ccfs_file_open(path, CCFS_READ);
    if(!f) return NULL;
    void *data = ccfs_file_read(f, size);
    fclose(f);
    return data;
}
