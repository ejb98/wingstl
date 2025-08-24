/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#ifndef ENGINE_H
#define ENGINE_H

#include <stdbool.h>
#include <stdlib.h>

#include "types.h"

float get_aspect_ratio(const wing3d *wing);
float get_camber(float x, float m, float p);
float get_gradient(float x, float m, float p);
float get_thickness(float x, float t, bool is_closed);
float get_x_surf(float xc, float thickness, float theta, bool is_upper);
float get_z_surf(float zc, float thickness, float theta, bool is_upper);

size_t get_num_pts(const wing3d *wing);
size_t get_num_tris(const wing3d *wing);
size_t get_upper_ind(const wing3d *wing, int i, int j);
size_t get_lower_ind(const wing3d *wing, int i, int j);

size_t fill_upper_lower_inds(const wing3d *wing, size_t k, size_t *inds);
size_t fill_port_star_inds(const wing3d *wing, size_t k, size_t *inds);
size_t fill_aft_inds(const wing3d *wing, size_t k, size_t *inds);

vec3d *make_pts(const wing3d *wing);
size_t *make_inds(const wing3d *wing);

bool tip_overlaps(const wing3d *wing);

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
