//===--------------------------------------------------------------------------------------------===
// message.h - description
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Messages are just small data types passed around on MT queues
/// The idea is that we can decouple systems through the use of events and the
/// message box/memory slots.
///
/// We want events to be small. At the moment, it should be 64 bits,
/// which means it can be passed as value pretty much anywhere we're running on 64-bit platroms.
typedef struct ccmsg_s {
    uint8_t kind;
    union {
        int64_t i64;
        uint64_t u64;
        int32_t i32;
        uint32_t u32;
        float f32;
        char str[4];
    };
} ccmsg_t;

#ifdef __cplusplus
} /* extern "C" */
#endif
