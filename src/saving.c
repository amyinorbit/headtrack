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

#include <acfutils/helpers.h>
#include <acfutils/log.h>
#include <acfutils/safe_alloc.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <setjmp.h>

jmp_buf exc;
const char *exc_msg;


static const htk_settings_t defaults = {
    .axes_invert = {true, false, false, false, false, true},
    .axes_sens = {2, 2, 2, 2, 2, 0.5},
    .rotation_smooth = .5f,
    .translation_smooth = .5f,
    .input_smooth = .5f
};

static const char *axes_sensitivity_name[] = {
    "x_sensitivity", "y_sensitivity", "z_sensitivity",
    "yaw_sensitivity", "pitch_sensitivity", "roll_sensitivity",
};

static const char *axes_reverse_name[] = {
    "x_reversed", "y_reversed", "z_reversed",
    "yaw_reversed", "pitch_reversed", "roll_reversed",
};


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

static void settings_load_from(const char *filename) {
    cJSON_Hooks hooks;
    hooks.malloc_fn = safe_malloc;
    hooks.free_fn = free;
    cJSON_InitHooks(&hooks);
    logMsg("loading settings from `%s`", filename);

    char *json_data = NULL;
    cJSON *json = NULL;

    if(!setjmp(exc)) {
        json_data = file2str(filename, NULL);
        if(!json_data) fail("unable to read json file");
        json = cJSON_Parse(json_data);
        if(!json) fail("invalid json file");

        cJSON *axes = cJSON_GetObjectItem(json, "axes");
        if(!axes || !cJSON_IsObject(axes)) fail("missing axes data");
        for(int i = 0; i < 6; ++i) {
            htk_settings.axes_sens[i] = get_f64(axes, axes_sensitivity_name[i]);
            htk_settings.axes_invert[i] = get_b(axes, axes_reverse_name[i]);
        }

        cJSON *smoothing = cJSON_GetObjectItem(json, "smoothing");
        if(!axes || !cJSON_IsObject(axes)) fail("missing smoothing data");
        htk_settings.input_smooth = get_f64(smoothing, "input_smoothing");
        htk_settings.rotation_smooth = get_f64(smoothing, "exp_rotation");
        htk_settings.translation_smooth = get_f64(smoothing, "exp_translation");
        
        logMsg("settings loaded");
    } else {
        logMsg("configuration parsing error: %s", exc_msg);
    }

    

    if(json_data) free(json_data);
    if(json) cJSON_Delete(json);
}

void settings_load_global() {
    char *path = mkpathname(xpath_plugin(), "config.json", NULL);
    if(file_exists(path, NULL)) {
        settings_load_from(path);
    } else {
        logMsg("no global settings found, using defaults");
        htk_settings = defaults;
    }
    free(path);
    htk_settings_did_update();
}

bool settings_load_plane() {
    char *path = mkpathname(xpath_aircraft(), "htrack.json", NULL);
    if(!file_exists(path, NULL)) {
        logMsg("no plane-specific settings found");
        free(path);
        return false;
    }
    settings_load_from(path);
    free(path);
    return true;
}

bool settings_save(bool global) {
    cJSON_Hooks hooks;
    hooks.malloc_fn = safe_malloc;
    hooks.free_fn = lacf_free;
    cJSON_InitHooks(&hooks);

    char *path = global
        ? mkpathname(xpath_plugin(), "config.json", NULL)
        : mkpathname(xpath_aircraft(), "config.json", NULL);

    logMsg("saving settings to `%s`", path);

    cJSON *obj = cJSON_CreateObject();
    cJSON *axes = cJSON_AddObjectToObject(obj, "axes");
    for(int i = 0; i < 6; ++i) {
        cJSON_AddNumberToObject(axes, axes_sensitivity_name[i], htk_settings.axes_sens[i]);
        cJSON_AddBoolToObject(axes, axes_reverse_name[i], htk_settings.axes_invert[i]);
    }

    cJSON *smoothing = cJSON_AddObjectToObject(obj, "smoothing");
    cJSON_AddNumberToObject(smoothing, "input_smoothing", htk_settings.input_smooth);
    cJSON_AddNumberToObject(smoothing, "exp_rotation", htk_settings.rotation_smooth);
    cJSON_AddNumberToObject(smoothing, "exp_translation", htk_settings.translation_smooth);

    if(!setjmp(exc)) {
        FILE *out = fopen(path, "rb");
        if(!out) fail("unable to write json file");

        char *json_data = cJSON_Print(obj);
        fputs(json_data, out);
        fclose(out);
        free(json_data);
        cJSON_Delete(obj);
        free(path);
    } else {
        logMsg("configuration save error: %s", exc_msg);
        free(path);
        return false;
    }
    logMsg("settings saved");
    return true;
}
