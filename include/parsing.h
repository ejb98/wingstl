/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#ifndef PARSING_H
#define PARSING_H

int handle_chord_pts(int iarg, int num_args, char **args);
int handle_inputs(int num_args, char **args, wing_props *wing);

float handle_semi_span(int iarg, int num_args, char **args);
float handle_root_chord(int iarg, int num_args, char **args);
float handle_sweep(int iarg, int num_args, char **args, const char *arg_flag);

int handle_airfoil(int iarg, int num_args, char **args);

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
