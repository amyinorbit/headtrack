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

#include <XPLMGraphics.h>
#include <XPLMMenus.h>
#include <ccore/log.h>
#include <stdbool.h>
#include <stdlib.h>
#include <tgmath.h>
#include <pthread.h>
#include <string.h>

struct {
    bool is_enabled;
    bool is_failed;
    bool must_reset;
    bool plane_spec;

    double viewport_ref[3];
    double head_in[6]; // reported by UDP
    double head[6]; // What we send to x-plane
    double neutral[6];

    struct {
        XPLMDataRef view_type;

        XPLMDataRef ref_x;
        XPLMDataRef ref_y;
        XPLMDataRef ref_z;

        XPLMDataRef head_x;
        XPLMDataRef head_y;
        XPLMDataRef head_z;

        XPLMDataRef head_pit;
        XPLMDataRef head_hdg;
        XPLMDataRef head_rll;

        XPLMDataRef headshake;
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

htk_settings_t htk_settings;

const char *htk_cmd_toggle = "amyinorbit/htrack/toggle";
const char *htk_cmd_center_head = "amyinorbit/htrack/center_head";
const char *htk_cmd_center_sim = "amyinorbit/htrack/center_sim";


void htk_setup() {
    htk_settings.last_error = NULL;
    settings_load_global();
    // htk_settings = defaults;

    state.is_failed = false;
    state.is_enabled = false;
    state.must_reset = true;
    state.plane_spec = false;

    state.cmd.toggle = XPLMCreateCommand(htk_cmd_toggle, "toggle head tracking");
    CCASSERT(state.cmd.toggle);
    state.cmd.center_head_tracking = XPLMCreateCommand(htk_cmd_center_head, "recenter head tracking");
    CCASSERT(state.cmd.center_head_tracking);
    state.cmd.center_sim_view = XPLMCreateCommand(htk_cmd_center_sim, "recenter sim view");
    CCASSERT(state.cmd.center_sim_view);

    state.dr.view_type = NULL;

    state.dr.ref_x = NULL;
    state.dr.ref_y = NULL;
    state.dr.ref_z = NULL;

    state.dr.head_x = NULL;
    state.dr.head_y = NULL;
    state.dr.head_z = NULL;

    state.dr.head_pit = NULL;
    state.dr.head_hdg = NULL;
    state.dr.head_rll = NULL;

    state.dr.headshake = NULL;

    state.menu.id = 0;
    state.menu.enabled = -1;
    state.menu.home = -1;
    state.menu.settings = -1;
}

static XPLMDataRef find_dref_checked(const char *path, bool strict) {
    XPLMDataRef dr = XPLMFindDataRef(path);
    if(!dr) {
        if(strict) {
            CCERROR("dataref `%s` not found", path);
            state.is_failed = true;
            abort();
        } else {
            CCINFO("optional dataref `%s` not found", path);
        }
    } else {
        CCINFO("dataref `%s` found", path);
    }
    return dr;
}

static int toggle_cb(XPLMCommandRef cmd, XPLMCommandPhase phase, void *refcon) {
    CCUNUSED(cmd);
    CCUNUSED(refcon);
    if(phase != xplm_CommandBegin) return 1;

    state.is_enabled = !state.is_enabled;
    // Reflect tracking statein the menu.
    XPLMCheckMenuItem(
        state.menu.id,
        state.menu.enabled,
        state.is_enabled  ? xplm_Menu_Checked : xplm_Menu_NoCheck
    );
    CCINFO("head tracking is %s", state.is_enabled ? "on" : "off");
    if(state.dr.headshake) {
        XPLMSetDatai(state.dr.headshake, state.is_enabled);
    }
    return 1;
}

static int center_head_cb(XPLMCommandRef cmd, XPLMCommandPhase phase, void *refcon) {
    CCUNUSED(cmd);
    CCUNUSED(refcon);
    if(phase != xplm_CommandBegin) return 1;

    for(int i = 0; i < 6; ++i) state.neutral[i] = state.head_in[i];
    CCINFO("saved neutral head position");
    return 1;
}

static int center_sim_cb(XPLMCommandRef cmd, XPLMCommandPhase phase, void *refcon) {
    CCUNUSED(cmd);
    CCUNUSED(refcon);
    if(phase != xplm_CommandBegin) return 1;
    state.viewport_ref[0] = XPLMGetDataf(state.dr.head_x);
    state.viewport_ref[1] = XPLMGetDataf(state.dr.head_y);
    state.viewport_ref[2] = XPLMGetDataf(state.dr.head_z);
    CCINFO("saved pilot's head location");
    return 1;
}

static void menu_cb(void *menu, void *refcon) {
    CCUNUSED(menu);
    CCUNUSED(refcon);
    settings_show();
}

void htk_start() {
    CCINFO("finding plane rotation datarefs");

    for(int i = 0; i < 6; ++i) {
        state.head_in[i] = 0.0;
        state.neutral[i] = 0.0;
    }

    CCINFO("installing command handler");
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
    state.menu.settings = XPLMAppendMenuItem(state.menu.id, "Settings...", NULL, 0);

    CCINFO("Setting up datarefs");

    state.dr.view_type = find_dref_checked("sim/graphics/view/view_type", true);
    state.dr.ref_x = find_dref_checked("sim/aircraft/view/acf_peX", true);
    state.dr.ref_y = find_dref_checked("sim/aircraft/view/acf_peY", true);
    state.dr.ref_z = find_dref_checked("sim/aircraft/view/acf_peZ", true);

    state.dr.head_pit = find_dref_checked("sim/graphics/view/pilots_head_the", true);
    state.dr.head_hdg = find_dref_checked("sim/graphics/view/pilots_head_psi", true);
    state.dr.head_rll = find_dref_checked("sim/graphics/view/pilots_head_phi", true);

    state.dr.head_x = find_dref_checked("sim/graphics/view/pilots_head_x", true);
    state.dr.head_y = find_dref_checked("sim/graphics/view/pilots_head_y", true);
    state.dr.head_z = find_dref_checked("sim/graphics/view/pilots_head_z", true);

    state.dr.headshake = find_dref_checked("simcoders/headshake/override", false);

    if(!server_start(state.head_in)) state.is_failed = true;
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

static double limits[6];

void htk_plane_did_load() {
    if(state.is_failed) return;
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
    CCINFO("recording default pilot's head position");
    state.viewport_ref[0] = XPLMGetDataf(state.dr.ref_x);
    state.viewport_ref[1] = XPLMGetDataf(state.dr.ref_y);
    state.viewport_ref[2] = XPLMGetDataf(state.dr.ref_z);
}

void htk_frame() {

    if(state.is_failed) return;
    if(state.must_reset) reload_plane();

    int view_type = XPLMGetDatai(state.dr.view_type);

    memcpy(state.head, state.head_in, sizeof(state.head));

    for(int i = 0; i < 6; ++i) {
        htk_settings.head[i] = state.head_in[i];
        state.head[i] -= state.neutral[i];
        if(htk_settings.axes_invert[i]) state.head[i] = -state.head[i];
    }
    if(view_type != 1026 || !state.is_enabled) return;

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

    XPLMSetDataf(state.dr.head_x, 1e-2 * state.head[0] + state.viewport_ref[0]);
    XPLMSetDataf(state.dr.head_y, 1e-2 * state.head[1] + state.viewport_ref[1]);
    XPLMSetDataf(state.dr.head_z, 1e-2 * state.head[2] + state.viewport_ref[2]);

    XPLMSetDataf(state.dr.head_hdg, normalize_rot(state.head[3]));
    XPLMSetDataf(state.dr.head_pit, normalize_rot(state.head[4]));
    XPLMSetDataf(state.dr.head_rll, normalize_rot(state.head[5]));
}
