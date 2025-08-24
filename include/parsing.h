/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#ifndef PARSING_H
#define PARSING_H

#include <stdbool.h>
#include <stdio.h>
#include "types.h"

#define MAX_SWEEP 90.0f
#define MIN_SWEEP 10.0f
#define MIN_CHORD_PTS 20
#define MAX_CHORD_PTS 200

#define FLAG_AIRFOIL "-a"
#define FLAG_SWEEP_LE "-le"
#define FLAG_SWEEP_TE "-te"
#define FLAG_CHORD_PTS "-p"
#define FLAG_SEMI_SPAN "-b"
#define FLAG_ROOT_CHORD "-c"

int handle_chord_pts(int iarg, int num_args, char **args);
int handle_inputs(int num_args, char **args, wing_props *wing);

float handle_semi_span(int iarg, int num_args, char **args);
float handle_root_chord(int iarg, int num_args, char **args);
float handle_sweep(int iarg, int num_args, char **args, const char *arg_flag);

bool wing_tip_overlaps(const wing_props *wing);
naca4 handle_airfoil(int iarg, int num_args, char **args);

#endif

/*
 * ------------------------------------------------------------------------
 * This file is part of wingstl.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ------------------------------------------------------------------------
 */
