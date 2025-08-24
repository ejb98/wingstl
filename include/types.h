/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

typedef struct vec3 {
    float x, y, z;
} vec3;

typedef struct naca4 {
    int m, p, t;
} naca4;

typedef struct wing_props {
    naca4 airfoil;

    int num_pts_span;
    int num_pts_chord;

    float semi_span;
    float root_chord;
    float sweep_angles[2];

    bool has_closed_te;
    bool has_cosine_spacing;
} wing_props;

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