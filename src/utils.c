/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#include <math.h>
#include <float.h>
#include <stdlib.h>

#include "types.h"
#include "constants.h"

float to_rads(float degrees) {
    return degrees * PI_OVER_180;
}

size_t sub2ind(int i, int j, int num_cols) {
    return (size_t) i * num_cols + j;
}

void cross(const vec3d *a, const vec3d *b, vec3d *v) {
    v->x = a->y * b->z - a->z * b->y;
    v->y = a->z * b->x - a->x * b->z;
    v->z = a->x * b->y - a->y * b->x;
}

void subtract(const vec3d *a, const vec3d *b, vec3d *v) {
    v->x = a->x - b->x;
    v->y = a->y - b->y;
    v->z = a->z - b->z;
}

void normalize(vec3d *v) {
    float d = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);

    if (d <= FLT_EPSILON) {
        return;
    }

    v->x /= d;
    v->y /= d;
    v->z /= d;
}

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