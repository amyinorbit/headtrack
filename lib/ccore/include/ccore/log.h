//===--------------------------------------------------------------------------------------------===
// log.h - Logging utilities for the UNS
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ccore/debug.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NDEBUG

#define CCDEBUG(fmt, ...)

#define CCASSERT(expr)

#else
#define CCDEBUG(...) cc_log(LOG_DEBUG, __FUNCTION__, __VA_ARGS__)
#define CCASSERT(expr) do { \
    if(!(expr)) { \
        CCERROR("assertion `" #expr "` failed\n(%s:%03d)", __FILE__, __LINE__); \
        cc_print_stack(); \
        abort(); \
    } \
} while(0)

#endif

#define CCINFO(...) cc_log(LOG_INFO, __func__, __VA_ARGS__)
#define CCWARN(...) cc_log(LOG_WARN, __func__, __VA_ARGS__)
#define CCERROR(...) cc_log(LOG_ERROR, __func__, __VA_ARGS__)

#define CCUNREACHABLE() (__builtin_unreachable())
#define CCUNUSED(v) ((void)(v))

typedef enum log_level_e {
    LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR
} log_level_t;

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_DEBUG
#endif

/// Sets the function used to print log messages. Defaults to fprintf(stderr, ...).
void cc_set_printer(void (*handler)(const char *));
void cc_set_log_name(const char *name);

/// Prints a log message at [level], in function [unit] at [line].
void cc_log(log_level_t level, const char *function, const char *fmt, ...);
// void cc_assert(bool expr, const char *readable, const char *file, const char *unit, int line);
void cc_print(const char *str);
void cc_printf(const char *str, ...);

#ifdef __cplusplus
} /* extern "C" */
#endif
