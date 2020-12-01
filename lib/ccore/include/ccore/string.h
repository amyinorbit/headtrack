//===--------------------------------------------------------------------------------------------===
// string - Ccore string utilities
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <string.h>
#include <ccore/log.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))

// Following from Saso Kiselkov
/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.0 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
 *
 * You can obtain a copy of the license in the file COPYING
 * or http://www.opensource.org/licenses/CDDL-1.0.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file COPYING.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2020 Saso Kiselkov. All rights reserved.
 */
static inline void
string_copy(char *restrict dest, const char *restrict src, size_t cap)
{
	size_t l;

	CCASSERT(cap != 0);
	/*
	 * We MUSTN'T use strlen here, because src may be SIGNIFICANTLY
	 * larger than dest and we don't want to measure the ENTIRE body
	 * of src. We only care for length UP TO the destination capacity.
	 */
	for (l = 0; l + 1 < cap && src[l] != '\0'; l++)
		;
	/*
	 * Due to a bug in GCC, we can't use strncpy, as it sometimes throws
	 * "call to __builtin___strncpy_chk will always overflow destination
	 * buffer", even when it's absolutely NOT the case.
	 */
	memcpy(dest, src, MIN(cap - 1, l + 1));
	/* Insure the string is ALWAYS terminated */
	dest[cap - 1] = '\0';
}

#ifdef __cplusplus
} /* extern "C" */
#endif
