//===--------------------------------------------------------------------------------------------===
// list.c - Doubly-linked list implementation
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <ccore/list.h>
#include <ccore/memory.h>
#include <ccore/log.h>

static inline cclist_node_t *get_node(const void *ptr) {
    return (cclist_node_t*)ptr;
}

static inline void *get_ptr(const cclist_node_t *node) {
    return (void *)node;
}

void cclist_init(cclist_t *list) {
    CCASSERT(list);
    list->size = 0;
    list->head.next = &list->head;
    list->head.prev = &list->head;
}

void cclist_clear(cclist_t *list, cc_destructor des, void *user_data) {
    CCASSERT(list);
    for(cclist_node_t *item = list->head.next; item != &list->head;) {
        cclist_node_t *to_free = item;
        item = item->next;
        if(des) des(to_free, user_data);
    }
    cclist_init(list);
}

static inline cclist_node_t *cclist_nth_item(cclist_t *list, size_t n) {
    cclist_node_t *item = list->head.next;
    while(item != &list->head && n--) {
        item = item->next;
    }
    CCASSERT(!n);
    return item;
}

static void cclist_do_insert_after(cclist_t *list, void* object, cclist_node_t *node) {
    cclist_node_t *item = object;

    item->prev = node;
    item->next = node->next;
    node->next->prev = item;
    node->next = item;
    list->size += 1;
}

static void cclist_do_insert_before(cclist_t *list, void* object, cclist_node_t *node) {
    cclist_node_t *item = object;

    item->next = node;
    item->prev = node->prev;
    node->prev->next = item;
    node->prev = item;
    list->size += 1;
}

void cclist_insert_at(cclist_t *list, void *object, size_t n) {
    CCASSERT(list);
    cclist_node_t *nth = cclist_nth_item(list, n);
    cclist_do_insert_before(list, object, nth);
}

void cclist_insert_first(cclist_t *list, void *object) {
    CCASSERT(list);
    cclist_do_insert_after(list, object, &list->head);
}

void cclist_insert_last(cclist_t *list, void *object) {
    CCASSERT(list);
    cclist_do_insert_before(list, object, &list->head);
}

void cclist_insert_after(cclist_t *list, void *object, const void *item) {
    CCASSERT(list);
    CCASSERT(item);
    cclist_node_t *node = get_node(item);
    cclist_do_insert_after(list, object, node);
}

void cclist_insert_before(cclist_t *list, void *object, const void *item) {
    CCASSERT(list);
    CCASSERT(item);
    cclist_node_t *node = get_node(item);
    cclist_do_insert_before(list, object, node);
}

void cclist_remove(cclist_t *list, void *object) {
    CCASSERT(list);
    CCASSERT(object);
    cclist_node_t *node = get_node(object);

    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = node->next = NULL;
    list->size -= 1;
}

void cclist_remove_first(cclist_t *list) {
    CCASSERT(list);
    if(!list->size) return;

    list->head.next = list->head.next->next;
    list->head.next->prev = &list->head;
    list->size -= 1;
}

void cclist_remove_last(cclist_t *list) {
    CCASSERT(list);
    if(!list->size) return;

    list->head.prev = list->head.prev->prev;
    list->head.prev->next = &list->head;
    list->size -= 1;
}

void *cclist_first(const cclist_t *list) {
    CCASSERT(list);
    if(list->head.next == &list->head) return NULL;
    return get_ptr(list->head.next);
}

void *cclist_last(const cclist_t *list) {
    CCASSERT(list);
    if(list->head.prev == &list->head) return NULL;
    return get_ptr(list->head.prev);
}

void *cclist_next(const cclist_t *list, const void *current) {
    cclist_node_t *node = get_node(current);
    if(node->next == &list->head) return NULL;
    return get_ptr(node->next);
}

void *cclist_prev(const cclist_t *list, const void *current) {
    cclist_node_t *node = get_node(current);
    if(node->prev == &list->head) return NULL;
    return get_ptr(node->prev);
}
