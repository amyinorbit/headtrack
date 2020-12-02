//===--------------------------------------------------------------------------------------------===
// paths.h - X-Plane utility for getting paths (plugins, etc).
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

const char *xpath_system();
const char *xpath_plugin();
const char *xpath_aircraft();

void xpath_plugin_prefix(const char *file, char *out, int max);
void xpath_system_prefix(const char *file, char *out, int max);
void xpath_aircraft_prefix(const char *file, char *out, int max);

bool xpath_file_exists(const char *file);

#ifdef __cplusplus
} /* extern "C" */
#endif
