//===--------------------------------------------------------------------------------------------===
// tpool - thread pool implementation
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include "tpool.h"
#include <ccore/memory.h>
#include <ccore/log.h>

static tpool_t *pool = NULL;
static pthread_mutex_t single_mt = PTHREAD_MUTEX_INITIALIZER;

static void *pool_worker(void *refcon) {
    tpool_t *pool = refcon;
    CCASSERT(pool);

    for(;;) {
        pthread_mutex_lock(&pool->mt);
        while(!pool->tasks.size && !pool->stop) pthread_cond_wait(&pool->cv, &pool->mt);
        if(pool->stop) {
            pthread_mutex_unlock(&pool->mt);
            break;
        }

        task_t *task = cclist_first(&pool->tasks);
        cclist_remove_first(&pool->tasks);

        pool->in_work += 1;
        pthread_mutex_unlock(&pool->mt);

        CCASSERT(task);
        CCASSERT(task->fn);

        task->fn(task->refcon);
        cc_free(task);

        pthread_mutex_lock(&pool->mt);
        pool->in_work -= 1;
        pthread_cond_signal(&pool->idle_cv);
        pthread_mutex_unlock(&pool->mt);
    }
    return NULL;
}

void ccpool_start(int num_threads) {
    pthread_mutex_lock(&single_mt);
    if(!pool) {
        pool = cc_alloc(sizeof(tpool_t) + num_threads * sizeof(pthread_t));
        pool->thread_count = num_threads;
        pool->stop = false;
        pool->in_work = 0;
        cclist_init(&pool->tasks);
        pthread_mutex_init(&pool->mt, NULL);
        pthread_cond_init(&pool->cv, NULL);
        pthread_cond_init(&pool->idle_cv, NULL);

        for(int i = 0; i < num_threads; ++i) {
            pthread_create(&pool->workers[i], NULL, pool_worker, pool);
        }
    }
    pthread_mutex_unlock(&single_mt);
}

static void task_destructor(void *task, void *refcon) {
    CCUNUSED(refcon);
    cc_free(task);
}

void ccpool_stop() {
    pthread_mutex_lock(&single_mt);
    if(pool) {
        pthread_mutex_lock(&pool->mt);
        pool->stop = true;
        cclist_clear(&pool->tasks, task_destructor, NULL);
        pthread_cond_broadcast(&pool->cv);
        pthread_mutex_unlock(&pool->mt);

        for(uint8_t i = 0; i < pool->thread_count; ++i) {
            pthread_join(pool->workers[i], NULL);
        }
        cc_free(pool);
        pool = NULL;
    }
    pthread_mutex_unlock(&single_mt);
}

void ccpool_submit(ccpool_task_t fn, void *refcon) {
    CCASSERT(fn);
    CCASSERT(pool);
    task_t *task = cc_alloc(sizeof (task_t));
    task->fn = fn;
    task->refcon = refcon;

    pthread_mutex_lock(&pool->mt);
    cclist_insert_last(&pool->tasks, task);
    pthread_cond_signal(&pool->cv);
    pthread_mutex_unlock(&pool->mt);
}

void ccpool_wait() {
    CCASSERT(pool);
    pthread_mutex_lock(&pool->mt);
    while(pool->in_work || pool->tasks.size) pthread_cond_wait(&pool->idle_cv, &pool->mt);
    pthread_mutex_unlock(&pool->mt);
}
