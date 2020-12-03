//===--------------------------------------------------------------------------------------------===
// paths.c - Use XPLM to find relative paths
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include "paths.h"
#include <ccore/log.h>
#include <ccore/filesystem.h>
#include <stdbool.h>
#include <XPLMUtilities.h>
#include <XPLMPlugin.h>
#include <XPLMPlanes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static char plugin_path[1024];
static char xsystem_path[1024];
static char aircraft_path[1024];

void xpath_reload() {
    XPLMGetSystemPath(xsystem_path);
    XPLMPluginID id = XPLMGetMyID();
    XPLMGetPluginInfo(id, NULL, plugin_path, NULL, NULL);
    ccfs_path_rtrim_i(plugin_path, 1);
    CCINFO("plugin path: %s", plugin_path);

    char plane_name[512];
    XPLMGetNthAircraftModel(XPLM_USER_AIRCRAFT, plane_name, aircraft_path);
    ccfs_path_rtrim_i(aircraft_path, 0);
    CCINFO("aircraft path: %s", aircraft_path);
}

const char *xpath_system() {
    return xsystem_path;
}
const char *xpath_plugin() {
    return plugin_path;
}

const char *xpath_aircraft() {
    return aircraft_path;
}
