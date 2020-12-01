//===--------------------------------------------------------------------------------------------===
// log.c - logging utilities
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <ccore/log.h>
#include <stdio.h>
#include <pthread.h>
#include <ccore/math.h>
static pthread_mutex_t stream_mutex = PTHREAD_MUTEX_INITIALIZER;

static void default_printer(const char *str) {
    fprintf(stderr, "%s", str);
}

static void (*log_printer)(const char *) = default_printer;
static const char *log_name = "ccore";

static const char *level_string(log_level_t level) {
    switch(level) {
        case LOG_DEBUG: return "debug";
        case LOG_INFO: return "info";
        case LOG_WARN: return "warning";
        case LOG_ERROR: return "error";
    }
    return "<invalid>";
}

void cc_set_log_name(const char *name) {
    log_name = name;
}

void cc_set_printer(void (*printer)(const char *)) {
    log_printer = printer;
}

void cc_log(log_level_t level, const char *function, const char *fmt, ...) {
    if(level < LOG_LEVEL) return;
    pthread_mutex_lock(&stream_mutex);

    char buffer[512];
    snprintf(buffer, 512, "[%s %s] %s(): ", log_name, level_string(level), function);
    log_printer(buffer);

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 512, fmt, args);
    va_end(args);
    log_printer(buffer);
    log_printer("\n");
    pthread_mutex_unlock(&stream_mutex);
}

// void cc_assert(bool expr, const char *readable, const char *file, const char *unit, int line) {
//     if(!expr) {
//         cc_log(LOG_ERROR, unit, "assertion `%s` failed\n(%s:%03d)", readable, file, line);
//         cc_print_stack();
//         abort();
//     }
// }

void cc_print(const char *str) {
    pthread_mutex_lock(&stream_mutex);
    log_printer(str);
    pthread_mutex_unlock(&stream_mutex);
}
