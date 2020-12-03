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

void xpath_reload();

const char *xpath_system();
const char *xpath_plugin();
const char *xpath_aircraft();

#ifdef __cplusplus
} /* extern "C" */
#endif
