//===--------------------------------------------------------------------------------------------===
// format.h - various airac utilities
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

void format_date(time_t t, size_t n, char *out);
void format_hhmm(time_t t, size_t n, char *out);
void format_hhmmss(time_t t, size_t n, char *out);
void format_lat(float lat, size_t n, char *out);
void format_lon(float lat, size_t n, char *out);

bool is_number(const char *input);

#ifdef __cplusplus
} /* extern "C" */
#endif
