//===--------------------------------------------------------------------------------------------===
// math.h - simple inline maths utilities
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <tgmath.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline double normalize_rot(double hdg) {
    if(hdg < 180.0) hdg += 360.0;
    if(hdg >= 180.0) hdg -= 360.0;
    return hdg;
}

static inline double remapd(double v, double old_m, double new_m, double factor) {
    double old_n = v < 0 ? -pow(fabs(v) / old_m, factor) : pow(fabs(v) / old_m, factor);
    return old_n * new_m;
}

static inline void remapd3(double v[3], const double old_m[3], const double new_m[3], const double factor) {
    v[0] = remapd(v[0], old_m[0], new_m[0], factor);
    v[1] = remapd(v[1], old_m[1], new_m[1], factor);
    v[2] = remapd(v[2], old_m[2], new_m[2], factor);
}

static inline void normd3(double v[3]) {
    v[0] = normalize_rot(v[0]);
    v[1] = normalize_rot(v[1]);
    v[2] = normalize_rot(v[2]);
}

static inline double clampd(double v, double low, double high) {
    return v > high ? high : v < low ? low : v;
}

static inline double lerp(double a, double b, double t) {
    t = clampd(t, 0, 1);
    return a * (1.0 - t) + b * t;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
