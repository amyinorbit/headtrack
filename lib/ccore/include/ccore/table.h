//===--------------------------------------------------------------------------------------------===
// table.h - Bucket-based hash map.
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <ccore/list.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ccbucket_node_s {
    cclist_node_t base;
    union {
        cclist_t many_values;
        void *one_value;
    };
    char key[];
} ccbucket_node_t;

typedef struct ccbucket_value_s {
    cclist_node_t base;
    void *value;
} ccbucket_value_t;

/// A multi-valued, string-indexed hash table.
typedef struct cctable_s {
    size_t capacity;
    size_t size;
    bool allow_multiple;
    cclist_t *buckets;
} cctable_t;

/// Initialises a table and allocates memory for it. [size] should be close to the maximum
/// amount of items expected to be stored, so that items are evenly spread in the table.
void cctable_init(cctable_t *table, size_t count, bool allow_multiple);

/// De-initialises [table] and call [des] on its contents.
void cctable_deinit(cctable_t *table, cc_destructor des, void *user_data);

/// Maps [key] to [object] in [table].
void cctable_insert(cctable_t *table, const char *key, void *object);

/// Retrieves the entries mapped to [key] in [table].
const cclist_t *cctable_get_many(const cctable_t *table, const char *key);

/// Retrieves the entry mapped to [key] in [table].
void *cctable_get_one(const cctable_t *table, const char *key);

/// A function that can be used to iterate over a hash table.
typedef void (*cctable_callback_f)(const char *, const void *, void *);

/// Calls [callback] for each element stored in [table], with arbitrary data [ptr].
void cctable_iter(const cctable_t *table, cctable_callback_f callback, void *ptr);

#ifdef __cplusplus
} /* extern "C" */
#endif
