//===--------------------------------------------------------------------------------------------===
// tpool - private header for thread pool
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <ccore/list.h>
#include <ccore/tpool.h>
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct task_s {
    cclist_node_t base;
    ccpool_task_t fn;
    void *refcon;
} task_t;


typedef struct tpool_s {
    cclist_t tasks;
    bool stop;

    pthread_mutex_t mt;
    pthread_cond_t cv;
    pthread_cond_t idle_cv;

    uint8_t in_work;
    uint8_t thread_count;
    pthread_t workers[];
} tpool_t;
