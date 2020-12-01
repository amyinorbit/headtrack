//===--------------------------------------------------------------------------------------------===
// memory.h - Allocation functions for Ccore
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/// An allocator for C-core. Must behave like realloc().
typedef void *(*cc_allocator)(void *, size_t);

/// A destructor function
typedef void (*cc_destructor)(void *, void *);

extern void cc_default_destructor(void *ptr, void *);

/// Installs a custom memory allocation function for c-core function.
void cc_set_allocator(cc_allocator allocator);

/// Allocates [bytes] of memory and returns a pointer to it.
void *cc_alloc(size_t size);

/// Frees the memory at [ptr].
void cc_free(void *ptr);

/// Grows or shrink the memory at [ptr] so that at least [size] bytes are available.
void *cc_realloc(void *ptr, size_t size);

#ifdef __cplusplus
} /* extern "C" */
#endif
