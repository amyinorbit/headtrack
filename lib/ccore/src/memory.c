//===--------------------------------------------------------------------------------------------===
// memory.c - Ccore's memory allocation utilities
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <ccore/memory.h>
#include <ccore/log.h>
#include <stdlib.h>

static void *cc_default_alloc(void *ptr, size_t size) {
    if(!size) {
        if(ptr) free(ptr);
        return ptr;
    }
    ptr = realloc(ptr, size);
    CCASSERT(ptr);
    return ptr;
}

static cc_allocator __cc_alloc = cc_default_alloc;

void cc_default_destructor(void *ptr, void *unused) {
    cc_free(ptr);
}

void cc_set_allocator(cc_allocator allocator) {
    __cc_alloc = allocator;
}

void *cc_alloc(size_t size) {
    return __cc_alloc(NULL, size);
}

void cc_free(void *ptr) {
    __cc_alloc(ptr, 0);
}

void *cc_realloc(void *ptr, size_t size) {
    return __cc_alloc(ptr, size);
}
