//===--------------------------------------------------------------------------------------------===
// utils.c - various AIRAC and other utilities
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <ccore/format.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <tgmath.h>
#include <ctype.h>

void format_date(time_t t, size_t n, char *out) {
    struct tm *time_info = gmtime(&t);
    strftime(out, n, "%d-%b-%y", time_info);
}

void format_hhmm(time_t t, size_t n, char *out) {
    struct tm *time_info = gmtime(&t);
    strftime(out, n, "%R", time_info);
}

void format_hhmmss(time_t t, size_t n, char *out) {
    struct tm *time_info = gmtime(&t);
    strftime(out, n, "%T", time_info);
}

void format_lat(float lat, size_t n, char *out) {
    char hemisphere = lat < 0 ? 'S' : 'N';
    lat = fabsf(lat);
    float deg;
    float frac = modff(lat, &deg) * 60.f;
    snprintf(out, n, "%c  %02.0f %05.2f", hemisphere, deg, frac);
}

void format_lon(float lon, size_t n, char *out) {
    char hemisphere = lon < 0 ? 'W' : 'E';
    lon = fabsf(lon);
    float deg;
    float frac = modff(lon, &deg) * 60.f;
    snprintf(out, n, "%c %03.0f %05.2f", hemisphere, deg, frac);
}

bool is_number(const char *input) {
    while(*input) {
        if(!isdigit(*input)) return false;
        input += 1;
    }
    return true;
}
