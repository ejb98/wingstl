/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#ifndef PARSING_H
#define PARSING_H

#include "types.h"

void request_value(const char *desc, const char *flag);
void request_nonzero_positive(const char *desc, const char *flag);
void request_n_digits(const char *desc, const char *flag, int n);
void request_bounded_integer(const char *desc, const char *flag, int val, const char *qualifier);

int handle_chord_pts(int iarg, int num_args, char **args);
int handle_inputs(int num_args, char **args, Settings *settings);

float handle_sweep(int iarg, int num_args, char **args, const char *arg_flag);
float handle_nonzero_positive(int iarg, int num_args, char **args, const char *desc, const char *flag);

char *handle_output(int iarg, int num_args, char **args);

Units handle_units(int iarg, int num_args, char **args);
void handle_airfoil(int iarg, int num_args, char **args, Settings *settings);

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
