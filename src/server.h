//===--------------------------------------------------------------------------------------------===
// server.h - UDP server thread for head tracker
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool server_start(double *input);
void server_stop();
bool server_restart(double *input);

#ifdef __cplusplus
} /* extern "C" */
#endif
