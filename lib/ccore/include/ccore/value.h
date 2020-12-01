//===--------------------------------------------------------------------------------------------===
// value.h - vm-style value for UNS1
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ccore/log.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct object_s object_t;

/// A tagged union used when multiple types must be stored in a homogeneous container.
typedef struct value_s {
    enum { VALUE_NIL, VALUE_BOOL, VALUE_INT, VALUE_FLOAT, VALUE_STRING, VALUE_REF } kind;
    union {
        // TODO: move to new names:
        // uint32_t u32;
        // int32_t i32;
        // uint64_t u64;
        // int64_t i64;
        // float f32;
        // double f64;
        // const void *ptr;

        int64_t int_val;
        double float_val;
        const char *str_val;
        const void *ref_val;
    };
} value_t;

static inline value_t val_nil()
{ return (value_t){.kind=VALUE_NIL, .ref_val=NULL}; }

static inline value_t val_bool(bool b)
{ return (value_t){.kind=VALUE_BOOL, .int_val=((b) ? 1 : 0)}; }

static inline value_t val_int(int64_t i)
{ return (value_t){.kind=VALUE_INT, .int_val=(i)}; }

static inline value_t val_float(double f)
{ return (value_t){.kind=VALUE_FLOAT, .float_val=(f)}; }

static inline value_t val_string(const char *s)
{ return (value_t){.kind=VALUE_STRING, .str_val=(s)}; }

static inline value_t val_ref(const void *ref)
{ return (value_t){.kind=VALUE_REF, .ref_val=ref}; }

static inline bool as_bool(value_t value) {
    CCASSERT(value.kind == VALUE_BOOL);
    return value.int_val != 0;
}

static inline int64_t as_int(value_t value) {
    CCASSERT(value.kind == VALUE_INT);
    return value.int_val;
}

static inline double as_float(value_t value) {
    CCASSERT(value.kind == VALUE_FLOAT);
    return value.float_val;
}

static inline const char *as_string(value_t value) {
    CCASSERT(value.kind == VALUE_STRING);
    return value.str_val;
}

static inline const void *as_ref(value_t value) {
    CCASSERT(value.kind == VALUE_REF);
    return value.ref_val;
}

struct object_s {
    enum {OBJ_LEG, OBJ_STR} kind;
    object_t *next;
    bool is_marked;
};

typedef struct obj_gc_s {
    bool mark_flag;
    object_t *head;
    size_t allocated;
    size_t next_collect;
} obj_gc_t;

int val_compare_wpt(const void *ap, const void *bp);
int val_compare_string(const void *ap, const void *bp);

void gc_init(obj_gc_t *gc);
void gc_deinit(obj_gc_t *gc);
void gc_collect(obj_gc_t *gc);
void *gc_new(obj_gc_t *gc, size_t size, int kind);
void value_debug(value_t value);

#ifdef __cplusplus
} /* extern "C" */
#endif
