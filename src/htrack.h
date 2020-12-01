//===--------------------------------------------------------------------------------------------===
// look_at - the actual implementation of the look-at mechanism
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once


extern const char *htk_cmd_toggle;


typedef struct {
    union {
        struct { double x, y, z; };
        struct { double lat, lon, elev; };
        struct { double pit, hdg, rll; };
        double data[3];
    };
} vec3_t;


void htk_setup();
void htk_start();
void htk_stop();
void htk_cleanup();
void htk_frame();

void htk_reset_default_head();
