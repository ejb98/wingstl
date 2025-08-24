/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#include <stdio.h>
#include <stdbool.h>

#include "types.h"
#include "fileio.h"
#include "engine.h"
#include "parsing.h"
#include "messages.h"
#include "constants.h"
#include "validation.h"

int main(int argc, char **argv) {
    Wing wing = {
        .units = METERS,
        .airfoil = {DEFAULT_AIRFOIL},
        .semi_span = DEFAULT_SEMI_SPAN,
        .root_chord = DEFAULT_ROOT_CHORD,
        .sweep_angles = {DEFAULT_SWEEP_LE, DEFAULT_SWEEP_TE},
        .num_pts_span = DEFAULT_NUM_SPAN_PTS,
        .num_pts_chord = DEFAULT_NUM_CHORD_PTS,
        .has_closed_te = DEFAULT_HAS_CLOSED_TE,
        .has_cosine_spacing = DEFAULT_HAS_COSINE_SPACING
    };

    Settings settings = {.verbose = false};

    if (handle_inputs(argc, argv, &wing, &settings)) {
        return 0;
    }

    if (validate(&wing)) {
        return 0;
    }

    Vec3D *pts = make_pts(&wing);

    if (pts == NULL) {
        fprintf(stderr, "wingstl: error: unable to allocate memory for vertices\n");
        return 0;
    }

    size_t *indices = make_inds(&wing);

    if (indices == NULL) {
        fprintf(stderr, "wingstl: error: unable to allocate memory for triangle indices\n");
        free(pts);

        return 0;
    }

    char file_name[] = "wing.stl";

    if (write_stl(pts, indices, get_num_tris(&wing), file_name)) {
        fprintf(stderr, "wingstl: error: unable to open STL file for writing\n");
        free(indices);
        free(pts);

        return 0;
    }

    if (settings.verbose) {
        printf("STL file '%s' written successfully\n\n", file_name);
        show_wing_props(&wing);
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
