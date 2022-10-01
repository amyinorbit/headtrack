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
#include <jsmn_path.h>

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

static bool as_number(const char *json, const jsmntok_t *tok, float *out) {
    
    if(tok->type != JSMN_PRIMITIVE) return false;
    if(json[tok->start] != '+' && json[tok->start] != '-' && !isdigit(json[tok->start])) return false;
    
    *out = atof(&json[tok->start]);
    return true;
}

static bool as_bool(const char *json, const jsmntok_t *tok, bool *out) {
    
    if(tok->type != JSMN_PRIMITIVE) return false;
    if(json[tok->start] != 't' && json[tok->start] != 'f') return false;
    
    *out = (json[tok->start] == 't');
    return true;
}

static bool get_number(const char *json, const jsmntok_t *toks, int count, const char *path, float *out) {
    const jsmntok_t *tok = jsmn_path_lookup(json, toks, count, path);
    if(!tok) return false;
    return as_number(json, tok, out);
}

static void settings_load_from(const char *path) {
    
    size_t size = 0;
    char *json = file2buf(path, &size);
    if(!json) {
        logMsg("configuration error: cannot open %s", path);
        return;
    }
    logMsg("loading settings from `%s`", path);
    
    jsmn_parser parser;
    jsmntok_t toks[128];
    
    jsmn_init(&parser);
    int n_toks = jsmn_parse(&parser, json, size, toks, ARRAY_NUM_ELEM(toks));
    
    if(n_toks < 0) {
        logMsg("config error: invalid JSON file");
        goto errout;
    }
    
    for(int i = 0; i < 6; ++i) {
        const jsmntok_t *sens = jsmn_path_lookup_format(json, toks, n_toks,
            "axes/%s", axes_sensitivity_name[i]);
        const jsmntok_t *invert = jsmn_path_lookup_format(json, toks, n_toks,
            "axes/%s", axes_reverse_name[i]);
            
        if(!sens || !as_number(json, sens, &htk_settings.axes_sens[i])) {
            logMsg("missing or bad value for '%s'", axes_sensitivity_name[i]);
            goto errout;
        }
        
        if(!invert || !as_bool(json, invert, &htk_settings.axes_invert[i])) {
            logMsg("missing or bad value for '%s'", axes_reverse_name[i]);
            goto errout;
        }
    }
    
    if(!get_number(json, toks, n_toks,
        "smoothing/input_smoothing", &htk_settings.input_smooth)) goto errout;
    if(!get_number(json, toks, n_toks,
        "smoothing/exp_rotation", &htk_settings.rotation_smooth)) goto errout;
    if(!get_number(json, toks, n_toks,
        "smoothing/exp_translation", &htk_settings.translation_smooth)) goto errout;
errout:
    free(json);
    return;
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

static int level = 0;

static void indent(FILE *f) {
    for(int i = 0; i < level; ++i) {
        fprintf(f, "  ");
    }
}

static void start_obj(FILE *f, const char *name) {
    if(!name) {
        indent(f);
        fprintf(f, "{\n");
    } else {
        indent(f);
        fprintf(f, "\"%s\": {\n", name);
    }
    level += 1;
}

static void end_obj(FILE *f, bool last) {
    ASSERT(level > 0);
    level -= 1;
    indent(f);
    if(!last) {
        fprintf(f, "},\n");
    } else {
        fprintf(f, "}\n");
    }
}

static void json_bool(FILE *f, const char *key, bool val, bool last) {
    indent(f);
    fprintf(f, "\"%s\": %s%s\n", key, val ? "true" : "false", last ? "" : ",");
}

static void json_float(FILE *f, const char *key, float val, bool last) {
    indent(f);
    fprintf(f, "\"%s\": %f%s\n", key, val, last ? "" : ",");
}

bool settings_save(bool global) {
    char *path = global
        ? mkpathname(xpath_plugin(), "config.json", NULL)
        : mkpathname(xpath_aircraft(), "htrack.json", NULL);

    logMsg("saving settings to `%s`", path);
    
    FILE *out = fopen(path, "wb");
    if(!out) {
        logMsg("cannot write configuration file");
        free(path);
        return false;
    }
    
    level = 0;
    start_obj(out, NULL);
    start_obj(out, "axes");
    
    for(int i = 0; i < 6; ++i) {
        json_float(out, axes_sensitivity_name[i], htk_settings.axes_sens[i], false);
        json_bool(out, axes_reverse_name[i], htk_settings.axes_invert[i], i == 5);
    }
    end_obj(out, false);
    start_obj(out, "smoothing");
    json_float(out, "input_smoothing", htk_settings.input_smooth, false);
    json_float(out, "exp_rotation", htk_settings.rotation_smooth, false);
    json_float(out, "exp_translation", htk_settings.translation_smooth, true);
    end_obj(out, true);
    end_obj(out, true);
    
    fclose(out);
    free(path);
    logMsg("settings saved");
    return true;
}
