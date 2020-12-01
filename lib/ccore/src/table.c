//===--------------------------------------------------------------------------------------------===
// table.c - Multi-valued hash table implementation
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <ccore/table.h>
#include <ccore/log.h>
#include <ccore/memory.h>
#include <string.h>

static inline size_t next_power_of_2(size_t v) {
    v -= 1;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v += 1;
    return v;
}

static size_t hash_string(const char *string) {
    CCASSERT(string);
    //Fowler-Noll-Vo 1a hash
    // http://www.isthe.com/chongo/src/fnv/hash_64.c
    // http://create.stephan-brumme.com/fnv-hash/
    size_t hash = 0x84222325cbf29ce4ULL;
    for(size_t i = 0; string[i] != '\0'; ++i) {
        hash = (hash ^ string[i]) * 0x100000001b3ULL;
    }
    return hash;
}

void cctable_init(cctable_t *table, size_t count, bool allow_multiple) {
    CCASSERT(table);
    table->size = 0;
    table->capacity = next_power_of_2(count);
    table->buckets = cc_alloc(table->capacity * sizeof(cctable_t));
    table->allow_multiple = allow_multiple;

    for(size_t i = 0; i < table->capacity; ++i) {
        cclist_init(&table->buckets[i]);
    }
}

struct destructor_data {
    cc_destructor destructor;
    void *user_data;
};

static void node_destructor_single(void *ptr, void *meta) {
    ccbucket_node_t *node = ptr;
    struct destructor_data *data = meta;
    if(data->destructor) data->destructor(node->one_value, data->user_data);
    cc_free(node);
}

static void value_destructor_many(void *ptr, void *meta) {
    ccbucket_value_t *value = ptr;
    struct destructor_data *data = meta;
    if(data->destructor) data->destructor(value->value, data->user_data);
    cc_free(value);
}

static void node_destructor_many(void *ptr, void *meta) {
    ccbucket_node_t *node = ptr;
    struct destructor_data *data = meta;
    cclist_clear(&node->many_values, value_destructor_many, data);
    cc_free(node);
}

void cctable_deinit(cctable_t *table, cc_destructor des, void *ptr) {
    CCASSERT(table);

    struct destructor_data data;
    data.destructor = des;
    data.user_data = ptr;

    cc_destructor node_destructor = table->allow_multiple
        ? node_destructor_many
        : node_destructor_single;

    for(size_t i = 0; i < table->capacity; ++i) {
        cclist_clear(&table->buckets[i], node_destructor, &data);
    }
    cc_free(table->buckets);
    table->capacity = 0;
    table->size = 0;
    table->buckets = NULL;
}

static void cctable_insert_single(cctable_t *table, const char *key, void *object) {
    size_t key_length = strlen(key);

    size_t bucket_idx = hash_string(key) & (table->capacity-1);
    cclist_t *bucket = &table->buckets[bucket_idx];
    for(ccbucket_node_t *node = cclist_first(bucket); node; node = cclist_next(bucket, node)) {
        if(!strcmp(key, node->key)) return;
    }

    ccbucket_node_t *new_node = cc_alloc(sizeof(ccbucket_node_t) + key_length + 1);

    memcpy(new_node->key, key, key_length);
    new_node->key[key_length] = 0;
    new_node->one_value = object;
    cclist_insert_first(bucket, new_node);
    table->size += 1;
}

ccbucket_value_t *value_many_new(void *object) {
    ccbucket_value_t *value = cc_alloc(sizeof(ccbucket_value_t));
    value->value = object;
    return value;
}

static void cctable_insert_many(cctable_t *table, const char *key, void *object) {
    size_t key_length = strlen(key);

    size_t bucket_idx = hash_string(key) & (table->capacity-1);
    cclist_t *bucket = &table->buckets[bucket_idx];
    for(ccbucket_node_t *node = cclist_first(bucket); node; node = cclist_next(bucket, node)) {
        if(strcmp(key, node->key)) continue;
        table->size += 1;
        return cclist_insert_first(&node->many_values, value_many_new(object));
    }

    ccbucket_node_t *new_node = cc_alloc(sizeof(ccbucket_node_t) + key_length + 1);

    memcpy(new_node->key, key, key_length);
    new_node->key[key_length] = 0;
    cclist_init(&new_node->many_values);
    cclist_insert_first(bucket, new_node);
    table->size += 1;
    cclist_insert_first(&new_node->many_values, value_many_new(object));
}

void cctable_insert(cctable_t *table, const char *key, void *object) {
    CCASSERT(table);
    CCASSERT(key);
    if(table->allow_multiple) {
        cctable_insert_many(table, key, object);
    } else {
        cctable_insert_single(table, key, object);
    }
}

const cclist_t *cctable_get_many(const cctable_t *table, const char *key) {
    CCASSERT(table);
    CCASSERT(key);
    CCASSERT(table->allow_multiple);

    size_t bucket_idx = hash_string(key) & (table->capacity-1);
    cclist_t *bucket = &table->buckets[bucket_idx];

    for(ccbucket_node_t *node = cclist_first(bucket); node; node = cclist_next(bucket, node)) {
        if(strcmp(key, node->key)) continue;
        return &node->many_values;
    }
    return NULL;
}

void *cctable_get_one(const cctable_t *table, const char *key) {
    CCASSERT(table);
    CCASSERT(key);
    CCASSERT(!table->allow_multiple);

    size_t bucket_idx = hash_string(key) & (table->capacity-1);
    cclist_t *bucket = &table->buckets[bucket_idx];

    for(ccbucket_node_t *node = cclist_first(bucket); node; node = cclist_next(bucket, node)) {
        if(strcmp(key, node->key)) continue;
        return node->one_value;
    }
    return NULL;
}

void cctable_iter(const cctable_t *table, cctable_callback_f callback, void *ptr) {
    CCASSERT(table);
    CCASSERT(callback);

    for(size_t i = 0; i < table->capacity; ++i) {
        cclist_t *bucket = &table->buckets[i];
        for(ccbucket_node_t *n = cclist_first(bucket); n; n = cclist_next(bucket, n)) {
            if(!table->allow_multiple) {
                callback(n->key, n->one_value, ptr);
                continue;
            }
            for(ccbucket_value_t *value = cclist_first(&n->many_values);
                value;
                value = cclist_next(&n->many_values, value)) {
                callback(n->key, value->value, ptr);
            }
        }
    }
}
