/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#ifndef ENGINE_H
#define ENGINE_H

#include <stdbool.h>
#include <stdlib.h>

#include "types.h"

float get_surface_area(const Wing *wing);
float get_aspect_ratio(const Wing *wing);
float get_camber(float x, float m, float p);
float get_gradient(float x, float m, float p);
float get_thickness(float x, float t, bool is_closed);
float get_x_surface(float xc, float thickness, float theta, bool is_upper);
float get_z_surface(float zc, float thickness, float theta, bool is_upper);
float get_z_upper_selig(float xc, AirfoilData *airfoil);
float get_z_lower_selig(float xc, AirfoilData *airfoil);
float get_z_upper_lednicer(float xc, AirfoilData *airfoil);
float get_z_lower_lednicer(float xc, AirfoilData *airfoil);

size_t get_num_pts(const Wing *wing);
size_t get_num_tris(const Wing *wing);
size_t get_upper_index(const Wing *wing, int i, int j);
size_t get_lower_index(const Wing *wing, int i, int j);

size_t fill_upper_lower_indices(const Wing *wing, size_t k, size_t *inds);
size_t fill_port_star_indices(const Wing *wing, size_t k, size_t *inds);
size_t fill_aft_indices(const Wing *wing, size_t k, size_t *inds);

Vec3D *make_pts(const Wing *wing);
size_t *make_indices(const Wing *wing);

bool tip_overlap(const Wing *wing);

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
