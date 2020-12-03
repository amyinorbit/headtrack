//===--------------------------------------------------------------------------------------------===
// look_at - the actual implementation of the look-at mechanism
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <stdbool.h>
#include <pthread.h>
#include <XPLMUtilities.h>
#include <XPLMDataAccess.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    double axes_sens[6];
    bool axes_invert[6];
    float rotation_smooth;
    float translation_smooth;

    float input_smooth;

    float head[6];
    float sim[6];

    const char *last_error;
} htk_settings_t;
extern htk_settings_t htk_settings;

void htk_setup();
void htk_start();
void htk_stop();
void htk_cleanup();
void htk_frame();

void htk_settings_did_update();
void htk_plane_did_load();

void settings_show();
bool settings_is_visible();
void settings_cleanup();

bool settings_load_plane();
void settings_load_global();
bool settings_save(bool global);

#ifdef __cplusplus
} /* extern "C" */
#endif
