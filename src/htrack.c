//===--------------------------------------------------------------------------------------------===
// htrack.c - implementation of a basic OpenTrack protocol receiver.
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include "htrack.h"
#include <sys/types.h>
#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif
#include <unistd.h>
#include <string.h>

#include <XPLMGraphics.h>
#include <XPLMMenus.h>
#include <ccore/log.h>
#include <stdbool.h>
#include <stdlib.h>
#include <tgmath.h>
#include <pthread.h>

struct {
    bool is_enabled;
    bool is_failed;
    bool is_inside;
    bool server_is_running;
    bool must_reset;

    pthread_mutex_t lock;
    pthread_t server_thread;

    int server_socket;

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
    XPLMCommandRef toggle_cmd;
    XPLMCommandRef set_neutral_cmd;
} state;

htk_settings_t htk_settings;

const char *htk_cmd_toggle = "amyinorbit/htrack/toggle";
const char *htk_cmd_center = "amyinorbit/htrack/set_center";

static const htk_settings_t defaults = {
    .axes_invert = {true, false, false, false, false, true},
    .axes_limits = {50.f, 50.f, 50.f, 75.f, 75.f, 90.f},
    .rotation_smooth = .5f,
    .translation_smooth = .5f
};

void htk_setup() {
    htk_settings = defaults;

    state.is_failed = false;
    state.is_enabled = false;
    state.is_inside = false;
    state.server_is_running = false;
    state.must_reset = true;

    state.toggle_cmd = XPLMCreateCommand(htk_cmd_toggle, "toggle head tracking mode");
    CCASSERT(state.toggle_cmd);
    state.set_neutral_cmd = XPLMCreateCommand(htk_cmd_center, "set center head position");
    CCASSERT(state.set_neutral_cmd);

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
}

static XPLMDataRef find_dref_checked(const char *path, bool strict) {
    XPLMDataRef dr = XPLMFindDataRef(path);
    if(!dr) {
        CCERROR("dataref `%s` not found", path);
        state.is_failed = true;
        if(strict) abort();
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
    CCINFO("head tracking is %s", state.is_enabled ? "on" : "off");
    if(state.dr.headshake) {
        XPLMSetDatai(state.dr.headshake, state.is_enabled);
    }
    return 1;
}

static int set_neutral_cb(XPLMCommandRef cmd, XPLMCommandPhase phase, void *refcon) {
    CCUNUSED(cmd);
    CCUNUSED(refcon);
    if(phase != xplm_CommandBegin) return 1;

    for(int i = 0; i < 6; ++i) state.neutral[i] = state.head_in[i];
    CCINFO("saved neutral head position");
    return 1;
}

#include <errno.h>
#include <stdio.h>

static void *server(void * data) {
    CCUNUSED(data);
    CCINFO("starting head tracking server");


    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    state.server_socket = socket(PF_INET, SOCK_DGRAM, 0);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 250000;

    setsockopt(state.server_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4242);
    server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    // Bind the socket
    (void)bind(state.server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));

    CCINFO("head tracking server on");
    state.server_is_running = true;
    double udp_data[6];
    while(state.server_is_running) {
        ssize_t bytes = recvfrom(state.server_socket, (void*)udp_data, sizeof(udp_data), 0, NULL, NULL);
        if(bytes > 0) {
            for(int i = 0; i < 6; ++i) {
                state.head_in[i] = 0.9 * state.head_in[i] + 0.1 * udp_data[i];
            }
        }

    }

    CCINFO("shutting down head tracking server");
    close(state.server_socket);

    return NULL;
}

static void menu_cb(void *menu, void *refcon) {
    CCUNUSED(menu);

    switch((uint64_t)refcon) {
    case 3: settings_show(); break;
    default: break;
    }
}

void htk_start() {
    CCINFO("finding plane rotation datarefs");

    for(int i = 0; i < 6; ++i) {
        state.head_in[i] = 0.0;
        state.neutral[i] = 0.0;
    }

    int slot = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "HeadTrack", NULL, 0);
    XPLMMenuID htk_menu = XPLMCreateMenu("HeadTrack", XPLMFindPluginsMenu(), slot, menu_cb, NULL);
    XPLMAppendMenuItemWithCommand(htk_menu, "Toggle Head Tracking", state.toggle_cmd);
    XPLMAppendMenuItemWithCommand(htk_menu, "Set Neutral Head Position", state.set_neutral_cmd);
    XPLMAppendMenuItem(htk_menu, "Settings...", (void *)3, 0);

    state.dr.view_type = find_dref_checked("sim/graphics/view/view_type", true);
    state.dr.ref_x = find_dref_checked("sim/aircraft/view/acf_peX", true);
    state.dr.ref_y = find_dref_checked("sim/aircraft/view/acf_peY", true);
    state.dr.ref_z = find_dref_checked("sim/aircraft/view/acf_peZ", true);

    CCINFO("finding pilot's head rotation datarefs");
    state.dr.head_pit = find_dref_checked("sim/graphics/view/pilots_head_the", true);
    state.dr.head_hdg = find_dref_checked("sim/graphics/view/pilots_head_psi", true);
    state.dr.head_rll = find_dref_checked("sim/graphics/view/pilots_head_phi", true);

    state.dr.head_x = find_dref_checked("sim/graphics/view/pilots_head_x", true);
    state.dr.head_y = find_dref_checked("sim/graphics/view/pilots_head_y", true);
    state.dr.head_z = find_dref_checked("sim/graphics/view/pilots_head_z", true);

    state.dr.headshake = find_dref_checked("simcoders/headshake/override", false);

    CCINFO("installing command handler");
    XPLMRegisterCommandHandler(state.toggle_cmd, toggle_cb, 0, NULL);
    XPLMRegisterCommandHandler(state.set_neutral_cmd, set_neutral_cb, 0, NULL);

    CCINFO("starting head tracking server");
    pthread_create(&state.server_thread, NULL, server, NULL);

}

void htk_stop() {
    XPLMUnregisterCommandHandler(state.toggle_cmd, toggle_cb, 0, NULL);
    XPLMUnregisterCommandHandler(state.set_neutral_cmd, set_neutral_cb, 0, NULL);
    state.server_is_running = false;
    state.is_enabled = false;
    pthread_join(state.server_thread, NULL);
}

void htk_cleanup() {
    settings_cleanup();
}
/*
static inline double clampd(double v, double low, double high) {
    return v > high ? high : v < low ? low : v;
}
*/

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

static const double limits_out[6] = {100, 100, 100, 135, 90, 90};


void htk_reset_default_head() {
    if(state.is_failed) return;
    state.must_reset = true;
}


void htk_frame() {

    if(state.is_failed) return;

    if(state.must_reset) {
        state.must_reset = false;
        CCINFO("recording default pilot's head position");
        state.viewport_ref[0] = XPLMGetDataf(state.dr.ref_x);
        state.viewport_ref[1] = XPLMGetDataf(state.dr.ref_y);
        state.viewport_ref[2] = XPLMGetDataf(state.dr.ref_z);
    }

    int view_type = XPLMGetDatai(state.dr.view_type);
    state.is_inside = view_type == 1026;
    if(!state.is_inside || !state.is_enabled) return;

    memcpy(state.head, state.head_in, sizeof(state.head));

    for(int i = 0; i < 6; ++i) {
        state.head[i] -= state.neutral[i];
        if(htk_settings.axes_invert[i]) state.head[i] = -state.head[i];
    }

    remapd3(state.head,
        htk_settings.axes_limits,
        limits_out,
        1.f + htk_settings.translation_smooth);
    normd3(state.head + 3);
    remapd3(state.head + 3,
        htk_settings.axes_limits + 3,
        limits_out + 3,
        1.f + htk_settings.rotation_smooth);

    XPLMSetDataf(state.dr.head_x, 1e-2 * state.head[0] + state.viewport_ref[0]);
    XPLMSetDataf(state.dr.head_y, 1e-2 * state.head[1] + state.viewport_ref[1]);
    XPLMSetDataf(state.dr.head_z, 1e-2 * state.head[2] + state.viewport_ref[2]);

    XPLMSetDataf(state.dr.head_hdg, normalize_rot(state.head[3]));
    XPLMSetDataf(state.dr.head_pit, normalize_rot(state.head[4]));
    XPLMSetDataf(state.dr.head_rll, normalize_rot(state.head[5]));
}
