//===--------------------------------------------------------------------------------------------===
// value.c - value/gc system implementation
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <ccore/value.h>
#include <ccore/log.h>
#include <ccore/memory.h>
#include <stdio.h>

#define GC_INITIAL_THRESHOLD (128)

void gc_init(obj_gc_t *gc) {
    CCASSERT(gc);
    gc->mark_flag = true;
    gc->allocated = 0;
    gc->next_collect = GC_INITIAL_THRESHOLD;
    gc->head = NULL;
}

void gc_deinit(obj_gc_t *gc) {
    CCASSERT(gc);

    object_t *obj = gc->head;
    while(obj) {
        object_t *to_delete = obj;
        obj = obj->next;
        cc_free(to_delete);
    }

    // TODO: follow chain and deallocate
    gc_init(gc);
}

void gc_collect(obj_gc_t *gc) {
    // First, we be marking!
    object_t *obj = gc->head;
    while(obj) {

        obj = obj->next;
    }

}

void *gc_new(obj_gc_t *gc, size_t size, int kind) {
    CCASSERT(gc);
    CCASSERT(size > sizeof(object_t));
    object_t *obj = cc_alloc(size);

    gc->allocated += 1;
    if(gc->allocated > gc->next_collect) gc_collect(gc);

    obj->is_marked = !gc->mark_flag;
    obj->kind = kind;
    obj->next = gc->head;
    gc->head = obj;
    return obj;
}

void value_debug(value_t value) {
    switch (value.kind) {
        case VALUE_NIL:
            printf("<nil>\n");
            break;
        case VALUE_BOOL:
            printf("%s\n", as_bool(value) ? "true" : "false");
            break;

        case VALUE_INT:
            printf("%ld\n", as_int(value));
            break;

        case VALUE_FLOAT:
            printf("%f\n", as_float(value));
            break;

        case VALUE_STRING:
            printf("%s\n", as_string(value));
            break;

        case VALUE_REF:
            printf("<ref:%p>\n", as_ref(value));
            break;
    }
}
