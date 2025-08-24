/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#include <stdio.h>

#include "utils.h"
#include "types.h"
#include "engine.h"
#include "parsing.h"
#include "constants.h"

int main(int argc, char **argv) {
    wing_props wing = {
        .airfoil = {-1},
        .semi_span = -1.0f,
        .root_chord = -1.0f,
        .sweep_angles = {90.0f, 90.0f},
        .num_pts_span = NUM_SPAN_PTS,
        .num_pts_chord = MIN_CHORD_PTS,
        .has_closed_te = HAS_CLOSED_TE,
        .has_cosine_spacing = HAS_COSINE_SPACING
    };

    if (handle_inputs(argc, argv, &wing)) {
        return 0;
    }

    if (wing.airfoil.m < 0) {
        fprintf(stderr, "wingstl: error: please provide a four-digit number for the naca airfoil using flag '%s'\n", FLAG_AIRFOIL);

        return 0;
    }

    if (wing.semi_span < 0.0f) {
        fprintf(stderr, "wingstl: error: please provide a value for the semi span using flag '%s'\n", FLAG_SEMI_SPAN);
        
        return 0;
    }

    if (wing.root_chord < 0.0f) {
        fprintf(stderr, "wingstl: error: please provide a value for the root chord using flag '%s'\n", FLAG_ROOT_CHORD);
        
        return 0;
    }

    if (tip_overlaps(&wing)) {
        fprintf(stderr, "wingstl: error: wing tip overlap detected; ");
        fprintf(stderr, "try adjusting values for '%s', '%s', '%s' or '%s'\n", 
                FLAG_SWEEP_LE, FLAG_SWEEP_TE, FLAG_SEMI_SPAN, FLAG_ROOT_CHORD);

        return 0;
    }

    float aspect_ratio = get_aspect_ratio(&wing);

    if (aspect_ratio < 1.0f || aspect_ratio > 100.0f) {
        fprintf(stderr, "wingstl: error: extreme aspect ratio detected; ");
        fprintf(stderr, "try adjusting values for '%s', '%s', '%s' or '%s'\n", 
                FLAG_SWEEP_LE, FLAG_SWEEP_TE, FLAG_SEMI_SPAN, FLAG_ROOT_CHORD);

        return 0;
    }

    vec3 *pts = make_pts(&wing);

    if (pts == NULL) {
        fprintf(stderr, "wingstl: error: unable to allocate memory for surface vertices\n");

        return 0;
    }

    size_t *indices = make_inds(&wing);

    if (indices == NULL) {
        fprintf(stderr, "wingstl: error: unable to allocate memory for triangle indices\n");
        free(pts);

        return 0;
    }

    if (write_stl(pts, indices, get_num_tris(&wing), "wing.stl")) {
        fprintf(stderr, "wingstl: error: unable to open STL file for writing\n");
        free(indices);
        free(pts);

        return 0;
    }
    
    free(indices);
    free(pts);

    return 0;
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
