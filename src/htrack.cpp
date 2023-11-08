//===--------------------------------------------------------------------------------------------===
// htrack.c - implementation of a basic OpenTrack protocol receiver.
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include "htrack.h"
#include "server.h"
#include "math.h"
#include "X-Camera.h"

#include <XPLMGraphics.h>
#include <XPLMMenus.h>
#include <acfutils/assert.h>
#include <acfutils/dr.h>
#include <acfutils/log.h>

#include <stdbool.h>
#include <stdlib.h>
#include <tgmath.h>
#include <pthread.h>
#include <string.h>

struct {
    bool is_enabled;
    bool has_headshake;
    bool must_reset;
    bool plane_spec;

    double viewport_ref[3];
    double head_in[6]; // reported by UDP
    double head[6]; // What we send to x-plane
    double neutral[6];

    struct {
        dr_t view_type;

        dr_t ref_x;
        dr_t ref_y;
        dr_t ref_z;

        dr_t head_x;
        dr_t head_y;
        dr_t head_z;

        dr_t head_pit;
        dr_t head_hdg;
        dr_t head_rll;

        dr_t headshake;
    } dr;

    struct {
        XPLMCommandRef toggle;
        XPLMCommandRef center_head_tracking;
        XPLMCommandRef center_sim_view;
    } cmd;

    struct {
        XPLMMenuID id;
        int settings;
        int enabled;
        int home;
    } menu;
} state;

X_Camera xCamera;

htk_settings_t htk_settings;

const char *htk_cmd_toggle = "amyinorbit/htrack/toggle";
const char *htk_cmd_center_head = "amyinorbit/htrack/center_head";
const char *htk_cmd_center_sim = "amyinorbit/htrack/center_sim";


void htk_setup() {
    htk_settings.last_error = NULL;
    settings_load_global();
    // htk_settings = defaults;

    state.is_enabled = false;
    state.has_headshake = false;
    state.must_reset = true;
    state.plane_spec = false;

    state.cmd.toggle = XPLMCreateCommand(htk_cmd_toggle, "toggle head tracking");
    ASSERT(state.cmd.toggle);
    state.cmd.center_head_tracking = XPLMCreateCommand(htk_cmd_center_head, "recenter head tracking");
    ASSERT(state.cmd.center_head_tracking);
    state.cmd.center_sim_view = XPLMCreateCommand(htk_cmd_center_sim, "recenter sim view");
    ASSERT(state.cmd.center_sim_view);

    logMsg("Setting up datarefs");

    fdr_find(&state.dr.view_type, "sim/graphics/view/view_type");
    fdr_find(&state.dr.ref_x, "sim/aircraft/view/acf_peX");
    fdr_find(&state.dr.ref_y, "sim/aircraft/view/acf_peY");
    fdr_find(&state.dr.ref_z, "sim/aircraft/view/acf_peZ");

    fdr_find(&state.dr.head_pit, "sim/graphics/view/pilots_head_the");
    fdr_find(&state.dr.head_hdg, "sim/graphics/view/pilots_head_psi");
    fdr_find(&state.dr.head_rll, "sim/graphics/view/pilots_head_phi");

    fdr_find(&state.dr.head_x, "sim/graphics/view/pilots_head_x");
    fdr_find(&state.dr.head_y, "sim/graphics/view/pilots_head_y");
    fdr_find(&state.dr.head_z, "sim/graphics/view/pilots_head_z");

    state.menu.id = 0;
    state.menu.enabled = -1;
    state.menu.home = -1;
    state.menu.settings = -1;
}


static int toggle_cb(XPLMCommandRef cmd, XPLMCommandPhase phase, void *refcon) {
    UNUSED(cmd);
    UNUSED(refcon);
    if(phase != xplm_CommandBegin) return 1;

    state.is_enabled = !state.is_enabled;
    // Reflect tracking statein the menu.
    XPLMCheckMenuItem(
        state.menu.id,
        state.menu.enabled,
        state.is_enabled  ? xplm_Menu_Checked : xplm_Menu_NoCheck
    );
    logMsg("head tracking is %s", state.is_enabled ? "on" : "off");
    if(state.has_headshake) {
        dr_seti(&state.dr.headshake, state.is_enabled);
    }
    return 1;
}

static int center_head_cb(XPLMCommandRef cmd, XPLMCommandPhase phase, void *refcon) {
    UNUSED(cmd);
    UNUSED(refcon);
    if(phase != xplm_CommandBegin) return 1;

    for(int i = 0; i < 6; ++i) state.neutral[i] = state.head_in[i];
    logMsg("saved neutral head position");
    return 1;
}

static int center_sim_cb(XPLMCommandRef cmd, XPLMCommandPhase phase, void *refcon) {
    UNUSED(cmd);
    UNUSED(refcon);
    if(phase != xplm_CommandBegin) return 1;
    state.viewport_ref[0] = dr_getf(&state.dr.head_x);
    state.viewport_ref[1] = dr_getf(&state.dr.head_y);
    state.viewport_ref[2] = dr_getf(&state.dr.head_z);
    logMsg("saved pilot's head location");
    return 1;
}

static void menu_cb(void *menu, void *refcon) {
    UNUSED(menu);
    UNUSED(refcon);
    settings_show();
}

int htk_start() {
    logMsg("finding plane rotation datarefs");

    for(int i = 0; i < 6; ++i) {
        state.head_in[i] = 0.0;
        state.neutral[i] = 0.0;
    }

    logMsg("installing command handler");
    XPLMRegisterCommandHandler(state.cmd.toggle, toggle_cb, 0, NULL);
    XPLMRegisterCommandHandler(state.cmd.center_head_tracking, center_head_cb, 0, NULL);
    XPLMRegisterCommandHandler(state.cmd.center_sim_view, center_sim_cb, 0, NULL);

    int slot = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "HeadTrack", NULL, 0);
    state.menu.id = XPLMCreateMenu("HeadTrack", XPLMFindPluginsMenu(), slot, menu_cb, NULL);
    state.menu.enabled = XPLMAppendMenuItemWithCommand(
        state.menu.id, "Track Head Motion", state.cmd.toggle);
    state.menu.home = XPLMAppendMenuItemWithCommand(
        state.menu.id, "Recenter Head Tracking", state.cmd.center_head_tracking);
    XPLMAppendMenuItemWithCommand(
        state.menu.id, "Recenter Sim View", state.cmd.center_sim_view);
    state.menu.settings = XPLMAppendMenuItem(state.menu.id, "Settings…", NULL, 0);

    state.has_headshake = dr_find(&state.dr.headshake, "simcoders/headshale/override");
    return server_start(state.head_in);
}

void htk_stop() {
    XPLMDestroyMenu(state.menu.id);
    XPLMUnregisterCommandHandler(state.cmd.toggle, toggle_cb, 0, NULL);
    XPLMUnregisterCommandHandler(state.cmd.center_head_tracking, center_head_cb, 0, NULL);
    XPLMUnregisterCommandHandler(state.cmd.center_sim_view, center_sim_cb, 0, NULL);
    state.is_enabled = false;
    server_stop();
}

void htk_cleanup() {
    settings_cleanup();
}


void htk_plane_did_load() {
    state.must_reset = true;
}
static const double limits_out[6] = {100, 100, 100, 135, 90, 90};
static double limits[6] = {50, 50, 50, 60, 60, 90};

void htk_settings_did_update() {
    for(int i = 0; i < 6; ++i) {
        limits[i] = limits_out[i] / htk_settings.axes_sens[i];
    }
}

static void reload_plane() {
    if(!settings_load_plane()) {
        if(state.plane_spec) settings_load_global();
        state.plane_spec = false;
    } else {
        state.plane_spec = true;
    }
    state.must_reset = false;
    logMsg("recording default pilot's head position");
    state.viewport_ref[0] = dr_getf(&state.dr.ref_x);
    state.viewport_ref[1] = dr_getf(&state.dr.ref_y);
    state.viewport_ref[2] = dr_getf(&state.dr.ref_z);
}

void htk_frame() {

    if(state.must_reset) reload_plane();

    int view_type = dr_geti(&state.dr.view_type);

    memcpy(state.head, state.head_in, sizeof(state.head));

    for(int i = 0; i < 6; ++i) {
        htk_settings.head[i] = state.head_in[i];
        state.head[i] -= state.neutral[i];
        if(htk_settings.axes_invert[i]) state.head[i] = -state.head[i];
    }
    
    if(!state.is_enabled)
    {
        return;
    }
    else if(xCamera.isEnabled())
    {
        // If X-Camera is present and enabled then we need to be
        // in either the 3D cockpit view or the free camera view (X-Camera external cameras)
        if(view_type != 1026 && view_type != 1028)
        {
            return;
        }
    }
    else if(view_type != 1026)
    {
        return;
    }

    remapd3(state.head,
        limits,
        limits_out,
        1.f + htk_settings.translation_smooth);
    normd3(state.head + 3);
    remapd3(state.head + 3,
        limits + 3,
        limits_out + 3,
        1.f + htk_settings.rotation_smooth);

    for(int i = 0; i < 6; ++i) {
        htk_settings.sim[i] = state.head[i];
    }
    
    if(xCamera.isEnabled())
    {
        HeadData data;
        data.X = 1e-2 * state.head[0];
        data.Y = 1e-2 * state.head[1];
        data.Z = 1e-2 * state.head[2];
        data.Yaw = normalize_rot(state.head[3]);
        data.Pitch = normalize_rot(state.head[4]);
        data.Roll = normalize_rot(state.head[5]);
        xCamera.setOffsets(data);

        return;
    }
    
    dr_setf(&state.dr.head_x, 1e-2 * state.head[0] + state.viewport_ref[0]);
    dr_setf(&state.dr.head_y, 1e-2 * state.head[1] + state.viewport_ref[1]);
    dr_setf(&state.dr.head_z, 1e-2 * state.head[2] + state.viewport_ref[2]);

    dr_setf(&state.dr.head_hdg, normalize_rot(state.head[3]));
    dr_setf(&state.dr.head_pit, normalize_rot(state.head[4]));
    dr_setf(&state.dr.head_rll, normalize_rot(state.head[5]));
}
