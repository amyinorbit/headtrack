//===--------------------------------------------------------------------------------------------===
// math.h - math utilities somehow not part of the standard library
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <math.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

static inline uint32_t max_u32(uint32_t a, uint32_t b) { return a > b ? a : b; }
static inline uint32_t min_u32(uint32_t a, uint32_t b) { return a < b ? a : b; }
static inline int32_t max_i32(int32_t a, int32_t b) { return a > b ? a : b; }
static inline int32_t min_i32(int32_t a, int32_t b) { return a < b ? a : b; }
static inline float max_f32(float a, float b) { return a > b ? a : b; }
static inline float min_f32(float a, float b) { return a < b ? a : b; }
static inline uint32_t clamp_u32(uint32_t v, uint32_t min, uint32_t max) {
    return v > min ? (v < max ? v : max) : min;
}
static inline int32_t clamp_i32(int32_t v, int32_t min, int32_t max) {
    return v > min ? (v < max ? v : max) : min;
}
static inline float clamp_f32(float v, float min, float max) {
    return v > min ? (v < max ? v : max) : min;
}

/// Returns the maximum of [a] and [b].
#define max(a, b) _Generic((a), uint32_t: max_u32, int32_t: max_i32, float: max_f32)(a, b)

/// Returns the minimum of [a] and [b].
#define min(a, b) _Generic((a), uint32_t: min_u32, int32_t: min_i32, float: min_f32)(a, b)

/// Clamps [v] to the range [hi, lo]
#define clamp(v, a, b) _Generic((v), uint32_t: clamp_u32, int32_t: clamp_i32, float: clamp_f32)(v, a, b)

#ifdef __cplusplus
} /* extern "C" */
#endif
