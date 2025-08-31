/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include "types.h"

size_t sub2ind(int i, int j, int num_cols);

int num_digits_in(int value);

float interp(Vec2D *p0, Vec2D *p2, float x);
float to_meters(float value, Units units);
float to_radians(float degrees);

Units to_units(const char *str);

void rstrip(char *line);
void cross(const Vec3D *a, const Vec3D *b, Vec3D *v);
void subtract(const Vec3D *a, const Vec3D *b, Vec3D *v);
void normalize(Vec3D *v);

bool nearly_equal(float a, float b);

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