/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>
#include <math.h>

#include "utils.h"
#include "types.h"
#include "constants.h"

float get_camber(float x, float m, float p) {
    float a = 2.0f * p * x - x * x;

    if (x < p && p > FLT_EPSILON) {
        return m * a / (p * p);
    }

    float b = 1.0f - p;

    return m * (1.0f - 2.0f * p + a) / (b * b);
}

float get_gradient(float x, float m, float p) {
    float a = (2.0f * m) * (p - x);

    if (x < p && p > FLT_EPSILON) {
        return a / (p * p);
    }

    float b = 1.0f - p;

    return a / (b * b);
}

float get_thickness(float x, float t, bool is_closed) {
    float x2 = x * x;
    float a4 = is_closed ? A4_CLOSED: A4_OPEN;  

    return (A0 * powf(x, 0.5f) + A1 * x + A2 * x2 + 
            A3 * x2 * x + a4 * x2 * x2) * t / 0.2f;
}

float get_x_surface(float xc, float thickness, float theta, bool is_upper) {
    float sign = is_upper ? -1.0f : 1.0f;

    return xc + sign * thickness * sinf(theta);
}

float get_z_surface(float zc, float thickness, float theta, bool is_upper) {
    float sign = is_upper ? 1.0f : -1.0f;

    return zc + sign * thickness * cosf(theta);
}

size_t get_num_pts(const Wing *wing) {
    return (size_t) wing->num_pts_span * (2 * wing->num_pts_chord - wing->has_closed_te - 1);
}

size_t get_num_tris(const Wing *wing) {
    size_t num_tris_surf = (wing->num_pts_chord - 1) * (wing->num_pts_span - 1) * 2;
    size_t num_tris_side = 2 * wing->num_pts_chord - wing->has_closed_te - 3;
    size_t num_tris_aft = !wing->has_closed_te * (wing->num_pts_span - 1) * 2;
    
    return 2 * (num_tris_surf + num_tris_side) + num_tris_aft;
}

Vec3D *make_pts(const Wing *wing) {
    Vec3D *pts = (Vec3D *) malloc(get_num_pts(wing) * sizeof(Vec3D));

    if (pts == NULL) {
        return NULL;
    }
    
    float dx_te;
    float dx_le;
    float theta;
    float xn_surf;
    float zn_surf;
    float y_camber;
    float xn_camber;
    float zn_camber;
    float thickness;
    float local_chord;

    int row_max = wing->num_pts_chord;
    int num_rows = wing->num_pts_chord;
    int num_cols = wing->num_pts_span;
    int row_start = 0;

    float m = wing->airfoil.m / 100.0f;
    float p = wing->airfoil.p / 10.0f;
    float t = wing->airfoil.t / 100.0f;
    float tan_le = tanf(to_radians(90.0f - wing->sweep_angles[0]));
    float tan_te = tanf(to_radians(90.0f - wing->sweep_angles[1]));

    size_t ind;

    for (int is_upper = 1; is_upper >= 0; is_upper--) {
        if (!is_upper) {
            row_start = 1;
            row_max = num_rows - wing->has_closed_te;
        }

        for (int j = 0; j < num_cols; j++) {
            y_camber = wing->semi_span * j / (num_cols - 1);
            dx_te = y_camber * tan_te;
            dx_le = y_camber * tan_le;
            local_chord = wing->root_chord + dx_te - dx_le;

            for (int i = row_start; i < row_max; i++) {
                if (is_upper) {
                    ind = sub2ind(i - row_start, j, num_cols);
                } else {
                    ind = (size_t) num_rows * num_cols + sub2ind(i - row_start, j, num_cols);
                }

                xn_camber = (float) i / (num_rows - 1);

                if (wing->has_cosine_spacing) {
                    xn_camber = (1.0f - cosf(xn_camber * PI)) / 2.0f;
                }

                theta = get_gradient(xn_camber, m, p);
                thickness = get_thickness(xn_camber, t, wing->has_closed_te);
                zn_camber = get_camber(xn_camber, m, p);
                zn_surf = get_z_surface(zn_camber, thickness, theta, is_upper);
                xn_surf = get_x_surface(xn_camber, thickness, theta, is_upper);

                pts[ind].y = to_meters(y_camber, wing->units);
                pts[ind].z = to_meters(zn_surf * local_chord, wing->units);
                pts[ind].x = to_meters(xn_surf * local_chord + dx_le, wing->units);
            }
        }
    }

    return pts;
}

size_t get_upper_index(const Wing *wing, int i, int j) {
    return sub2ind(i, j, wing->num_pts_span);
}

size_t get_lower_index(const Wing *wing, int i, int j) {
    bool is_last_row = (i == wing->num_pts_chord - 1);

    if (i == 0 || (is_last_row && wing->has_closed_te)) {
        return sub2ind(i, j, wing->num_pts_span);
    }

    size_t offset = sub2ind(i - 1, j, wing->num_pts_span);
    return (size_t) wing->num_pts_chord * wing->num_pts_span + offset;
}

size_t fill_upper_lower_indices(const Wing *wing, size_t k, size_t *inds) {
    size_t corners[4];

    for (int is_upper = 1; is_upper >= 0; is_upper--) {
        for (int i = 0; i < wing->num_pts_chord - 1; i++) {
            for (int j = 0; j < wing->num_pts_span - 1; j++) {
                if (is_upper) {
                    corners[0] = get_upper_index(wing, i, j);
                    corners[1] = get_upper_index(wing, i, j + 1);
                    corners[2] = get_upper_index(wing, i + 1, j + 1);
                    corners[3] = get_upper_index(wing, i + 1, j);
                } else {
                    corners[0] = get_lower_index(wing, i, j + 1);
                    corners[1] = get_lower_index(wing, i, j);
                    corners[2] = get_lower_index(wing, i + 1, j);
                    corners[3] = get_lower_index(wing, i + 1, j + 1);
                }

                inds[k++] = corners[3];
                inds[k++] = corners[2];
                inds[k++] = corners[1];
                inds[k++] = corners[3];
                inds[k++] = corners[1];
                inds[k++] = corners[0];
            }
        }
    }

    return k;
}

size_t fill_port_star_indices(const Wing *wing, size_t k, size_t *inds) {
    int j;
    bool is_last_row;
    size_t corners[4];

    for (int is_port = 1; is_port >= 0; is_port--) { 
        j = !is_port * (wing->num_pts_span - 1);

        for (int i = 0; i < wing->num_pts_chord - 1; i++) {
            is_last_row = (i == wing->num_pts_chord - 2);

            if (i == 0) {
                inds[k++] = get_upper_index(wing, i, j);

                if (is_port) {
                    inds[k++] = get_lower_index(wing, i + 1, j);
                    inds[k++] = get_upper_index(wing, i + 1, j);
                } else {
                    inds[k++] = get_upper_index(wing, i + 1, j);
                    inds[k++] = get_lower_index(wing, i + 1, j);
                }
            } else if (is_last_row && wing->has_closed_te) {
                inds[k++] = get_lower_index(wing, i + 1, j);

                if (is_port) {
                    inds[k++] = get_upper_index(wing, i, j);
                    inds[k++] = get_lower_index(wing, i, j);
                } else {
                    inds[k++] = get_lower_index(wing, i, j);
                    inds[k++] = get_upper_index(wing, i, j);
                }
            } else {
                if (is_port) {
                    corners[0] = get_lower_index(wing, i, j);
                    corners[1] = get_lower_index(wing, i + 1, j);
                    corners[2] = get_upper_index(wing, i + 1, j);
                    corners[3] = get_upper_index(wing, i, j);
                } else {
                    corners[0] = get_lower_index(wing, i + 1, j);
                    corners[1] = get_lower_index(wing, i, j);
                    corners[2] = get_upper_index(wing, i, j);
                    corners[3] = get_upper_index(wing, i + 1, j);
                }

                inds[k++] = corners[0];
                inds[k++] = corners[1];
                inds[k++] = corners[2];
                inds[k++] = corners[0];
                inds[k++] = corners[2];
                inds[k++] = corners[3];
            }
        }
    }

    return k;
}

size_t fill_aft_indices(const Wing *wing, size_t k, size_t *inds) {
    int i = wing->num_pts_chord - 1;
    size_t corners[4];

    for (int j = 0; j < wing->num_pts_span - 1; j++) {
        corners[0] = get_lower_index(wing, i, j);
        corners[1] = get_lower_index(wing, i, j + 1);
        corners[2] = get_upper_index(wing, i, j + 1);
        corners[3] = get_upper_index(wing, i, j);

        inds[k++] = corners[0];
        inds[k++] = corners[1];
        inds[k++] = corners[2];
        inds[k++] = corners[0];
        inds[k++] = corners[2];
        inds[k++] = corners[3];
    }

    return k;
}

size_t *make_indices(const Wing *wing) {
    size_t k = 0;
    size_t num_tris = get_num_tris(wing);
    size_t *indices = (size_t *) malloc(3 * num_tris * sizeof(size_t));

    if (indices == NULL) {
        return NULL;
    }

    k = fill_upper_lower_indices(wing, k, indices);
    k = fill_port_star_indices(wing, k, indices);

    if (!wing->has_closed_te) {
        k = fill_aft_indices(wing, k, indices);
    }

    size_t num_tris_created = k / 3;
    assert(num_tris_created == num_tris);

    return indices;
}

float get_surface_area(const Wing *wing) {
    float dx_le = wing->semi_span * tanf(to_radians(90.0f - wing->sweep_angles[0]));
    float dx_te = wing->semi_span * tanf(to_radians(90.0f - wing->sweep_angles[1]));
    
    return 2.0f * wing->root_chord * wing->semi_span + wing->semi_span * (dx_te - dx_le);
}

float get_aspect_ratio(const Wing *wing) {
    float s = get_surface_area(wing);
    float b = 2.0f * wing->semi_span;

    return (s > FLT_EPSILON) ? b * b / s : 0.0f;
}

bool tip_overlap(const Wing *wing) {
    float offsets[2];

    for (int i = 0; i < 2; i++) {
        offsets[i] = wing->semi_span * (to_radians(90.0f - wing->sweep_angles[i]));
    }

    return wing->root_chord + offsets[1] <= offsets[0];
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
