//===--------------------------------------------------------------------------------------------===
// array.h - Contiguous dynamically allocated array
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <ccore/memory.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ccarray_s {
    size_t capacity;
    size_t count;
    void **data;
} ccarray_t;

///
void ccarray_init(ccarray_t *array);

///
void ccarray_deinit(ccarray_t *array);

///
void ccarray_clear(ccarray_t *array, cc_destructor des, void *ptr);

///
void ccarray_add_first(ccarray_t *array);

///
void ccarray_add_last(ccarray_t *array);

#ifdef __cplusplus
} /* extern "C" */
#endif
