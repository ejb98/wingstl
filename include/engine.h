/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#ifndef ENGINE_H
#define ENGINE_H

#include <stdbool.h>
#include <stdlib.h>

#include "types.h"

float get_surface_area(const Settings *settings);
float get_aspect_ratio(const Settings *settings);
float get_naca4_camber(float x, float m, float p);
float get_naca4_gradient(float x, float m, float p);
float get_naca4_thickness(float x, float t, bool is_closed);
float get_naca4_surface_x(float xc, float thickness, float theta, bool is_upper);
float get_naca4_surface_z(float zc, float thickness, float theta, bool is_upper);
float get_selig_upper_z(float xc, Airfoil *airfoil);
float get_selig_lower_z(float xc, Airfoil *airfoil);
float get_lednicer_upper_z(float xc, Airfoil *airfoil);
float get_lednicer_lower_z(float xc, Airfoil *airfoil);
float get_lednicer_z(float xc, Airfoil *airfoil, bool is_upper);
float get_selig_z(float xc, Airfoil *airfoil, bool is_upper);
float get_surface_z(float xn_camber, Airfoil *airfoil, bool is_upper);
float get_surface_x(float xn_camber, Airfoil *airfoil, bool is_upper);

size_t get_num_pts(const Settings *settings);
size_t get_num_tris(const Settings *settings);
size_t get_upper_index(const Settings *settings, int i, int j);
size_t get_lower_index(const Settings *settings, int i, int j);

size_t fill_upper_lower_indices(const Settings *settings, size_t k, size_t *inds, int islice);
size_t fill_port_star_indices(const Settings *settings, size_t k, size_t *inds, int islice);
size_t fill_aft_indices(const Settings *settings, size_t k, size_t *inds, int islice);

Vec3D *make_pts(Settings *settings);

bool tip_overlap(const Settings *settings);

void adjust_and_scale(Airfoil *airfoil, float xmin, float xmax);
void assign_indices(const Settings *settings, size_t num_tris, int islice, size_t *indices);

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
