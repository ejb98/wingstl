/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <float.h>
#include <math.h>

#include "utils.h"
#include "types.h"
#include "fileio.h"
#include "engine.h"
#include "constants.h"

float get_selig_upper_z(float xc, Airfoil *airfoil) {
    float x;

    for (int i = 0; i < airfoil->num_pts; i++) {
        x = airfoil->pts[i].x;

        if (nearly_equal(x, xc)) {
            return airfoil->pts[i].y;
        }

        if (x < xc) {
            return interp(airfoil->pts + i, airfoil->pts + i - 1, xc);
        }
    }

    return 0.0f;
}

float get_selig_lower_z(float xc, Airfoil *airfoil) {
    int ifinal = airfoil->num_pts - 1;
    bool missing_point = !nearly_equal(airfoil->pts[0].x, airfoil->pts[ifinal].x);

    if (missing_point && nearly_equal(airfoil->pts[0].x, xc)) {
        return airfoil->pts[0].y;
    }

    float x;

    for (int i = airfoil->num_pts - 1; i >= 0; i--) {
        x = airfoil->pts[i].x;

        if (nearly_equal(x, xc)) {
            return airfoil->pts[i].y;
        }

        if (x < xc) {
            if (missing_point && i == ifinal) {
                return interp(airfoil->pts + i, airfoil->pts, xc);
            }

            return interp(airfoil->pts + i, airfoil->pts + i + 1, xc);
        }
    }

    return 0.0f;
}

float get_lednicer_upper_z(float xc, Airfoil *airfoil) {
    float x;

    for (int i = 0; i < airfoil->lednicer_index; i++) {
        x = airfoil->pts[i].x;

        if (nearly_equal(x, xc)) {
            return airfoil->pts[i].y;
        }

        if (x > xc) {
            if (i == 0) {
                float x_lower;

                for (int j = airfoil->lednicer_index; j < airfoil->num_pts; j++) {
                    x_lower = airfoil->pts[j].x;

                    if (nearly_equal(x_lower, xc)) {
                        return airfoil->pts[j].y;
                    }

                    if (x_lower < xc) {
                        return interp(airfoil->pts + j, airfoil->pts + j - 1, xc);
                    }
                }
            }

            return interp(airfoil->pts + i - 1, airfoil->pts + i, xc);
        }
    }

    return 0.0f;
}

float get_lednicer_lower_z(float xc, Airfoil *airfoil) {
    int ifinal = airfoil->num_pts - 1;
    int iupper_te = airfoil->lednicer_index - 1;

    bool missing_point = !nearly_equal(airfoil->pts[iupper_te].x, airfoil->pts[ifinal].x);

    if (missing_point && nearly_equal(airfoil->pts[iupper_te].x, xc)) {
        return airfoil->pts[iupper_te].y;
    }

    float x;

    for (int i = ifinal; i > iupper_te; i--) {
        x = airfoil->pts[i].x;

        if (nearly_equal(x, xc)) {
            return airfoil->pts[i].y;
        }

        if (x < xc) {
            if (missing_point && i == ifinal) {
                return interp(airfoil->pts + i, airfoil->pts + iupper_te, xc);
            }

            return interp(airfoil->pts + i, airfoil->pts + i + 1, xc);
        }
    }

    return 0.0f;
}

float get_lednicer_z(float xc, Airfoil *airfoil, bool is_upper) {
    return is_upper ? get_lednicer_upper_z(xc, airfoil) : get_lednicer_lower_z(xc, airfoil);
}

float get_selig_z(float xc, Airfoil *airfoil, bool is_upper) {
    return is_upper ? get_selig_upper_z(xc, airfoil) : get_selig_lower_z(xc, airfoil);
}

float get_naca4_camber(float x, float m, float p) {
    float a = 2.0f * p * x - x * x;

    if (x < p && p > FLT_EPSILON) {
        return m * a / (p * p);
    }

    float b = 1.0f - p;

    return m * (1.0f - 2.0f * p + a) / (b * b);
}

float get_naca4_gradient(float x, float m, float p) {
    float a = (2.0f * m) * (p - x);

    if (x < p && p > FLT_EPSILON) {
        return a / (p * p);
    }

    float b = 1.0f - p;

    return a / (b * b);
}

float get_naca4_thickness(float x, float t, bool is_closed) {
    float x2 = x * x;
    float a4 = is_closed ? A4_CLOSED: A4_OPEN;  

    return (A0 * powf(x, 0.5f) + A1 * x + A2 * x2 + 
            A3 * x2 * x + a4 * x2 * x2) * t / 0.2f;
}

float get_naca4_surface_x(float xc, float thickness, float theta, bool is_upper) {
    float sign = is_upper ? -1.0f : 1.0f;

    return xc + sign * thickness * sinf(theta);
}

float get_naca4_surface_z(float zc, float thickness, float theta, bool is_upper) {
    float sign = is_upper ? 1.0f : -1.0f;

    return zc + sign * thickness * cosf(theta);
}

float get_surface_x(float xn_camber, Airfoil *airfoil, bool is_upper) {
    if (airfoil->num_pts > 0) {
        return xn_camber;
    }

    float m = (airfoil->header[0] - '0') / 100.0f;
    float p = (airfoil->header[1] - '0') / 10.0f;
    float t = atoi(airfoil->header + 2) / 100.0f;
    float theta = get_naca4_gradient(xn_camber, m, p);
    float thickness = get_naca4_thickness(xn_camber, t, airfoil->has_closed_te);

    return get_naca4_surface_x(xn_camber, thickness, theta, is_upper);
}

float get_surface_z(float xn_camber, Airfoil *airfoil, bool is_upper) {
    if (airfoil->num_pts > 0 && airfoil->lednicer_index > 0) {
        return get_lednicer_z(xn_camber, airfoil, is_upper);
    }

    if (airfoil->num_pts > 0 && airfoil->lednicer_index == 0) {
        return get_selig_z(xn_camber, airfoil, is_upper);
    }

    float m = (airfoil->header[0] - '0') / 100.0f;
    float p = (airfoil->header[1] - '0') / 10.0f;
    float t = atoi(airfoil->header + 2) / 100.0f;
    float theta = get_naca4_gradient(xn_camber, m, p);
    float thickness = get_naca4_thickness(xn_camber, t, airfoil->has_closed_te);
    float zn_camber = get_naca4_camber(xn_camber, m, p);

    return get_naca4_surface_z(zn_camber, thickness, theta, is_upper);
}

size_t get_num_pts(const Settings *settings) {
    return (size_t) (settings->num_slices + 1) * (2 * settings->num_pts_chord - 
                     settings->airfoil.has_closed_te - 1);
}

size_t get_num_tris(const Settings *settings) {
    size_t num_tris_surf = (settings->num_pts_chord - 1) * 2;
    size_t num_tris_side = 2 * settings->num_pts_chord - settings->airfoil.has_closed_te - 3;
    size_t num_tris_aft = !settings->airfoil.has_closed_te * 2;
    
    return 2 * (num_tris_surf + num_tris_side) + num_tris_aft;
}

Vec3D *make_pts(Settings *settings) {
    Vec3D *pts = (Vec3D *) malloc(get_num_pts(settings) * sizeof(Vec3D));

    if (pts == NULL) {
        return NULL;
    }
    
    float dx_te;
    float dx_le;
    float xn_surf;
    float zn_surf;
    float y_camber;
    float xn_camber;
    float local_chord;

    int row_max = settings->num_pts_chord;
    int num_rows = settings->num_pts_chord;
    int num_cols = settings->num_slices + 1;
    int row_start = 0;

    float tan_le = tanf(to_radians(90.0f - settings->sweep_angles[0]));
    float tan_te = tanf(to_radians(90.0f - settings->sweep_angles[1]));

    size_t ind;

    for (int is_upper = 1; is_upper >= 0; is_upper--) {
        if (!is_upper) {
            row_start = 1;
            row_max = num_rows - settings->airfoil.has_closed_te;
        }

        for (int j = 0; j < num_cols; j++) {
            y_camber = settings->semi_span * j / (num_cols - 1);
            dx_te = y_camber * tan_te;
            dx_le = y_camber * tan_le;
            local_chord = settings->root_chord + dx_te - dx_le;

            for (int i = row_start; i < row_max; i++) {
                if (is_upper) {
                    ind = sub2ind(i - row_start, j, num_cols);
                } else {
                    ind = (size_t) num_rows * num_cols + sub2ind(i - row_start, j, num_cols);
                }

                xn_camber = (1.0f - cosf(((float) i / (num_rows - 1)) * PI)) / 2.0f;
                xn_surf = get_surface_x(xn_camber, &settings->airfoil, is_upper);
                zn_surf = get_surface_z(xn_camber, &settings->airfoil, is_upper);

                pts[ind].y = to_meters(y_camber, settings->units);
                pts[ind].z = to_meters(zn_surf * local_chord, settings->units);
                pts[ind].x = to_meters(xn_surf * local_chord + dx_le, settings->units);
            }
        }
    }

    return pts;
}

size_t get_upper_index(const Settings *settings, int i, int j) {
    return sub2ind(i, j, settings->num_slices + 1);
}

size_t get_lower_index(const Settings *settings, int i, int j) {
    bool is_last_row = (i == settings->num_pts_chord - 1);

    if (i == 0 || (is_last_row && settings->airfoil.has_closed_te)) {
        return sub2ind(i, j, settings->num_slices + 1);
    }

    size_t offset = sub2ind(i - 1, j, settings->num_slices + 1);
    return (size_t) settings->num_pts_chord * (settings->num_slices + 1) + offset;
}

size_t fill_upper_lower_indices(const Settings *settings, size_t k, size_t *inds, int islice) {
    size_t corners[4];

    int j = islice;
    for (int is_upper = 1; is_upper >= 0; is_upper--) {
        for (int i = 0; i < settings->num_pts_chord - 1; i++) {
            if (is_upper) {
                corners[0] = get_upper_index(settings, i, j);
                corners[1] = get_upper_index(settings, i, j + 1);
                corners[2] = get_upper_index(settings, i + 1, j + 1);
                corners[3] = get_upper_index(settings, i + 1, j);
            } else {
                corners[0] = get_lower_index(settings, i, j + 1);
                corners[1] = get_lower_index(settings, i, j);
                corners[2] = get_lower_index(settings, i + 1, j);
                corners[3] = get_lower_index(settings, i + 1, j + 1);
            }

            inds[k++] = corners[3];
            inds[k++] = corners[2];
            inds[k++] = corners[1];
            inds[k++] = corners[3];
            inds[k++] = corners[1];
            inds[k++] = corners[0];
        }
    }

    return k;
}

size_t fill_port_star_indices(const Settings *settings, size_t k, size_t *inds, int islice) {
    int j;
    bool is_last_row;
    size_t corners[4];

    for (int is_port = 1; is_port >= 0; is_port--) { 
        j = is_port ? islice : islice + 1;

        for (int i = 0; i < settings->num_pts_chord - 1; i++) {
            is_last_row = (i == settings->num_pts_chord - 2);

            if (i == 0) {
                inds[k++] = get_upper_index(settings, i, j);

                if (is_port) {
                    inds[k++] = get_lower_index(settings, i + 1, j);
                    inds[k++] = get_upper_index(settings, i + 1, j);
                } else {
                    inds[k++] = get_upper_index(settings, i + 1, j);
                    inds[k++] = get_lower_index(settings, i + 1, j);
                }
            } else if (is_last_row && settings->airfoil.has_closed_te) {
                inds[k++] = get_lower_index(settings, i + 1, j);

                if (is_port) {
                    inds[k++] = get_upper_index(settings, i, j);
                    inds[k++] = get_lower_index(settings, i, j);
                } else {
                    inds[k++] = get_lower_index(settings, i, j);
                    inds[k++] = get_upper_index(settings, i, j);
                }
            } else {
                if (is_port) {
                    corners[0] = get_lower_index(settings, i, j);
                    corners[1] = get_lower_index(settings, i + 1, j);
                    corners[2] = get_upper_index(settings, i + 1, j);
                    corners[3] = get_upper_index(settings, i, j);
                } else {
                    corners[0] = get_lower_index(settings, i + 1, j);
                    corners[1] = get_lower_index(settings, i, j);
                    corners[2] = get_upper_index(settings, i, j);
                    corners[3] = get_upper_index(settings, i + 1, j);
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

size_t fill_aft_indices(const Settings *settings, size_t k, size_t *inds, int islice) {
    int i = settings->num_pts_chord - 1;
    int j = islice;

    size_t corners[4];

    corners[0] = get_lower_index(settings, i, j);
    corners[1] = get_lower_index(settings, i, j + 1);
    corners[2] = get_upper_index(settings, i, j + 1);
    corners[3] = get_upper_index(settings, i, j);

    inds[k++] = corners[0];
    inds[k++] = corners[1];
    inds[k++] = corners[2];
    inds[k++] = corners[0];
    inds[k++] = corners[2];
    inds[k++] = corners[3];

    return k;
}

void assign_indices(const Settings *settings, size_t num_tris, int islice, size_t *indices) {
    size_t k = 0;

    k = fill_upper_lower_indices(settings, k, indices, islice);
    k = fill_port_star_indices(settings, k, indices, islice);

    if (!settings->airfoil.has_closed_te) {
        k = fill_aft_indices(settings, k, indices, islice);
    }

    size_t num_tris_created = k / 3;
    assert(num_tris_created == num_tris);
}

float get_surface_area(const Settings *settings) {
    float dx_le = settings->semi_span * tanf(to_radians(90.0f - settings->sweep_angles[0]));
    float dx_te = settings->semi_span * tanf(to_radians(90.0f - settings->sweep_angles[1]));
    
    return 2.0f * settings->root_chord * settings->semi_span + settings->semi_span * (dx_te - dx_le);
}

float get_aspect_ratio(const Settings *settings) {
    float s = get_surface_area(settings);
    float b = 2.0f * settings->semi_span;

    return (s > FLT_EPSILON) ? b * b / s : 0.0f;
}

bool tip_overlap(const Settings *settings) {
    float offsets[2];

    for (int i = 0; i < 2; i++) {
        offsets[i] = settings->semi_span * (to_radians(90.0f - settings->sweep_angles[i]));
    }

    return settings->root_chord + offsets[1] <= offsets[0];
}

void adjust_and_scale(Airfoil *airfoil, float xmin, float xmax) {
    float chord = xmax - xmin;
    float divisor = (chord > METERS_PER_MICROMETER) ? chord : 1.0f;
    
    for (int i = 0; i < airfoil->num_pts; i++) {
        airfoil->pts[i].x = (airfoil->pts[i].x - xmin)/divisor;
        airfoil->pts[i].y /= divisor;
    }

    int ite_lower = airfoil->num_pts - 1;
    int ite_upper = airfoil->lednicer_index > 0 ? airfoil->lednicer_index - 1 : 0;

    if (!nearly_equal(airfoil->pts[ite_upper].x, airfoil->pts[ite_lower].x)) {
        airfoil->has_closed_te = true;
    } else {
        airfoil->has_closed_te = nearly_equal(airfoil->pts[ite_upper].y, airfoil->pts[ite_lower].y);
    }
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
