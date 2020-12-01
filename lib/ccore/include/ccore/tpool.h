//===--------------------------------------------------------------------------------------------===
// tpool - thread pool for workers
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ccpool_task_t)(void *refcon);

void ccpool_start(int num_threads);
void ccpool_stop();

void ccpool_submit(ccpool_task_t task, void *refcon);
void ccpool_wait();

#ifdef __cplusplus
} /* extern "C" */
#endif
