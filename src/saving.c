//===--------------------------------------------------------------------------------------------===
// saving.c - Loading/saving settings for htrack
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include "htrack.h"
#include "paths.h"
#include <stdio.h>
#include <cjson/cJSON.h>
#include <ccore/log.h>
#include <ccore/memory.h>
#include <ccore/filesystem.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <setjmp.h>

jmp_buf exc;
const char *exc_msg;


static const htk_settings_t defaults = {
    .axes_invert = {true, false, false, false, false, true},
    .axes_limits = {50.f, 50.f, 50.f, 75.f, 75.f, 90.f},
    .rotation_smooth = .5f,
    .translation_smooth = .5f,
    .input_smooth = .5f
};

static const char *axes_limits_name[] = {
    "yaw_limit", "pitch_limit", "roll_limit",
    "x_limit", "y_limit", "z_limit"
};

static const char *axes_reverse_name[] = {
    "yaw_reversed", "pitch_reversed", "roll_reversed",
    "x_reversed", "y_reversed", "z_reversed"
};

static char *file_read(const char *path) {
    CCASSERT(path);
    FILE *in = ccfs_file_open(path, CCFS_READ);
    if(!in) return NULL;
    fseek(in, 0, SEEK_END);
    size_t length = ftell(in);
    fseek(in, 0, SEEK_SET);
    char *data = cc_alloc(length+1);
    if(fread(data, 1, length, in) != length) {
        CCWARN("invalid config read");
    }
    data[length] = '\0';
    fclose(in);
    return data;
}

static void fail(const char *message) {
    exc_msg = message;
    longjmp(exc, 2);
}

static bool get_b(cJSON *json, const char *name) {
    cJSON *field = cJSON_GetObjectItemCaseSensitive(json, name);
    if(!field || !cJSON_IsBool(field)) {
        fail("missing boolean in config file");
    }
    return field->valueint;
}

static double get_f64(cJSON *json, const char *name) {
    cJSON *field = cJSON_GetObjectItemCaseSensitive(json, name);
    if(!field || !cJSON_IsNumber(field)) {
        fail("missing boolean in config file");
    }
    return field->valuedouble;
}

static void load_settings_from(const char *filename) {
    cJSON_Hooks hooks;
    hooks.malloc_fn = cc_alloc;
    hooks.free_fn = cc_free;
    cJSON_InitHooks(&hooks);
    CCINFO("loading settings from `%s`", filename);

    char *json_data = NULL;
    cJSON *json = NULL;

    if(!setjmp(exc)) {
        json_data = file_read(filename);
        if(!json_data) fail("unable to read json file");
        json = cJSON_Parse(json_data);
        if(!json) fail("invalid json file");

        cJSON *axes = cJSON_GetObjectItem(json, "axes");
        if(!axes || !cJSON_IsObject(axes)) fail("missing axes data");
        for(int i = 0; i < 6; ++i) {
            htk_settings.axes_limits[i] = get_f64(axes, axes_limits_name[i]);
            htk_settings.axes_invert[i] = get_b(axes, axes_reverse_name[i]);
        }

        cJSON *smoothing = cJSON_GetObjectItem(json, "smoothing");
        if(!axes || !cJSON_IsObject(axes)) fail("missing smoothing data");
        htk_settings.input_smooth = get_f64(smoothing, "input_smoothing");
        htk_settings.rotation_smooth = get_f64(smoothing, "exp_rotation");
        htk_settings.translation_smooth = get_f64(smoothing, "exp_translation");

    } else {
        CCERROR("configuration parsing error: %s", exc_msg);
    }

    CCINFO("settings loaded");

    if(json_data) free(json_data);
    if(json) cJSON_Delete(json);
}

void settings_load(bool global) {

    char filename[1024];
    ccfs_path_concat(filename, sizeof(filename), xpath_aircraft(), "htrack.json", NULL);

    if(ccfs_path_exists(filename)) {
        load_settings_from(filename);
    } else if(global) {
        ccfs_path_concat(filename, sizeof(filename), xpath_plugin(), "config.json", NULL);
        if(ccfs_path_exists(filename)) {
            load_settings_from(filename);
        } else {
            CCINFO("No settings found, using defaults");
            htk_settings = defaults;
        }
    } else {
        CCINFO("No plane-specific settings found");
    }
}

bool settings_save(bool global) {
    cJSON_Hooks hooks;
    hooks.malloc_fn = cc_alloc;
    hooks.free_fn = cc_free;
    cJSON_InitHooks(&hooks);

    char filename[1024];
    if(global) {
        ccfs_path_concat(filename, sizeof(filename), xpath_plugin(), "config.json", NULL);
    } else {
        ccfs_path_concat(filename, sizeof(filename), xpath_aircraft(), "htrack.json", NULL);
    }

    CCINFO("saving settings to `%s`", filename);

    cJSON *obj = cJSON_CreateObject();
    cJSON *axes = cJSON_AddObjectToObject(obj, "axes");
    for(int i = 0; i < 6; ++i) {
        cJSON_AddNumberToObject(axes, axes_limits_name[i], htk_settings.axes_limits[i]);
        cJSON_AddBoolToObject(axes, axes_reverse_name[i], htk_settings.axes_invert[i]);
    }

    cJSON *smoothing = cJSON_AddObjectToObject(obj, "smoothing");
    cJSON_AddNumberToObject(smoothing, "input_smoothing", htk_settings.input_smooth);
    cJSON_AddNumberToObject(smoothing, "exp_rotation", htk_settings.rotation_smooth);
    cJSON_AddNumberToObject(smoothing, "exp_translation", htk_settings.translation_smooth);

    if(!setjmp(exc)) {
        FILE *out = ccfs_file_open(filename, CCFS_WRITE);
        if(!out) fail("unable to write json file");

        char *json_data = cJSON_Print(obj);
        fputs(json_data, out);
        fclose(out);
        cc_free(json_data);
        cJSON_Delete(obj);
    } else {
        CCERROR("configuration save error: %s", exc_msg);
        return false;
    }
    CCINFO("settings saved");
    return true;
}
