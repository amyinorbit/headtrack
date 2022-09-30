//===--------------------------------------------------------------------------------------------===
// paths.c - Use XPLM to find relative paths
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include "paths.h"
#include <acfutils/helpers.h>
#include <acfutils/log.h>
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

static void fix_path(char *path) {
    fix_pathsep(path);
    size_t len = strlen(path);
    VERIFY3U(len, !=, 0);
    if(path[len-1] == DIRSEP) {
        path[len-1] = '\0';
    }
}

void xpath_reload() {
    char name[512];
    XPLMGetSystemPath(xsystem_path);
	XPLMGetPluginInfo(XPLMGetMyID(), NULL, plugin_path, NULL, NULL);
    XPLMGetNthAircraftModel(XPLM_USER_AIRCRAFT, name, aircraft_path);
    
#if	IBM
	fix_pathsep(xsystem_path);
	fix_pathsep(plugin_path);
	fix_pathsep(aircraft_path);
#endif	/* IBM */
    
	/* cut off the trailing path component (our filename) */
    char *p = NULL;
    if((p = strrchr(aircraft_path, DIRSEP)) != NULL)
        *p = '\0';
    
    p = NULL;
	if ((p = strrchr(plugin_path, DIRSEP)) != NULL)
		*p = '\0';
	/*
	 * Cut off an optional '32' or '64' trailing component. Please note
	 * that XPLM 3.0 now supports OS-specific suffixes, so clamp those
	 * away as well.
	 */
	if ((p = strrchr(plugin_path, DIRSEP)) != NULL) {
		if (strcmp(p + 1, "64") == 0 || strcmp(p + 1, "32") == 0 ||
		    strcmp(p + 1, "win_x64") == 0 ||
		    strcmp(p + 1, "mac_x64") == 0 ||
		    strcmp(p + 1, "lin_x64") == 0)
			*p = '\0';
	}
    
    fix_path(plugin_path);
    fix_path(xsystem_path);
    fix_path(aircraft_path);
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
