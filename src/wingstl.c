/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#include <stdio.h>
#include <stdbool.h>

#include "utils.h"
#include "types.h"
#include "fileio.h"
#include "engine.h"
#include "parsing.h"
#include "messages.h"
#include "constants.h"
#include "validation.h"

int main(int argc, char **argv) {
    Settings settings = {
        .units = to_units(DEFAULT_UNITS),
        .airfoil = {DEFAULT_AIRFOIL},
        .semi_span = DEFAULT_SEMI_SPAN,
        .root_chord = DEFAULT_ROOT_CHORD,
        .sweep_angles = {DEFAULT_SWEEP_LE, DEFAULT_SWEEP_TE},
        .num_pts_span = DEFAULT_NUM_SPAN_PTS,
        .num_pts_chord = DEFAULT_NUM_CHORD_PTS,
        .verbose = false, .help = false, .output = NULL
    };

    if (handle_inputs(argc, argv, &settings)) {
        if (settings.help) {
            show_help();
            return 0;
        }
        
        return 1;
    }

    if (validate_settings(&settings)) {
        return 1;
    }

    Vec3D *pts = make_pts(&settings);

    if (pts == NULL) {
        fprintf(stderr, "wingstl: error: unable to allocate memory for vertices\n");
        return 1;
    }

    size_t *indices = make_indices(&settings);

    if (indices == NULL) {
        fprintf(stderr, "wingstl: error: unable to allocate memory for triangle indices\n");
        free(pts);

        return 1;
    }
    
    size_t num_tris = get_num_tris(&settings);

    if (write_stl(pts, indices, num_tris, !settings.output ? DEFAULT_OUTPUT : settings.output)) {
        free(settings.output);
        free(indices);
        free(pts);

        return 1;
    }

    if (settings.verbose) {
        printf("STL file written successfully\n\n");
        show_settings(&settings);
    }
    
    free(settings.output);
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
