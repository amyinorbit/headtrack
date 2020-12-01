//===--------------------------------------------------------------------------------------------===
// file.h - description
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <stddef.h>
#include <ccore/memory.h>

#ifdef __cplusplus
extern "C" {
#endif

/// A single linked list entry. Objects that can be inserted in a list should inherit from it.
typedef struct cclist_node_s {
    struct cclist_node_s *prev;
    struct cclist_node_s *next;
} cclist_node_t;

/// A basic doubly-linked list.
typedef struct cclist_s {
    size_t size;
    cclist_node_t head;
} cclist_t;

/// Initialises a list.
void cclist_init(cclist_t *list);

/// Clears [list] and calls [fn] on each item in it.
void cclist_clear(cclist_t *list, cc_destructor fn, void *user_data);

/// Inserts [object] into [list] at position [n].
void cclist_insert_at(cclist_t *list, void *object, size_t n);

/// Inserts [object] at the beginning of [list].
void cclist_insert_first(cclist_t *list, void *object);

/// Inserts [object] at the end of [list].
void cclist_insert_last(cclist_t *list, void *object);

/// Inserts [object] into [list] after [item].
void cclist_insert_after(cclist_t *list, void *object, const void *item);

/// Inserts [object] into [list] before [item].
void cclist_insert_before(cclist_t *list, void *object, const void *item);

/// Removes [object] from [list].
void cclist_remove(cclist_t *list, void *object);

/// Removes the first item from [list].
void cclist_remove_first(cclist_t *list);

/// Removes the last item from [list].
void cclist_remove_last(cclist_t *list);

/// Returns the first item in a list.
void *cclist_first(const cclist_t *list);

/// Returns the last item in [list].
void *cclist_last(const cclist_t *list);

/// Returns the item that comes after [node] in [list].
void *cclist_next(const cclist_t *list, const void *node);

/// Returns the item that comes before [node] in [list].
void *cclist_prev(const cclist_t *list, const void *node);
#ifdef __cplusplus
} /* extern "C" */
#endif
