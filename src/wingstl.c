/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
        .num_slices = DEFAULT_NUM_SLICES,
        .root_chord = DEFAULT_ROOT_CHORD,
        .sweep_angles = {DEFAULT_SWEEP_LE, DEFAULT_SWEEP_TE},
        .num_pts_chord = DEFAULT_NUM_CHORD_PTS,
        .verbose = false, .help = false, .output = NULL
    };

    if (handle_inputs(argc, argv, &settings) || validate_settings(&settings)) {
        if (settings.help) {
            show_help();
            free(settings.output);
            return 0;
        }
        
        free(settings.output);
        return 1;
    }

    Vec3D *pts = make_pts(&settings);

    if (pts == NULL) {
        fprintf(stderr, "wingstl: error: unable to allocate memory for vertices\n");
        free(settings.output);
        return 1;
    }

    size_t len_output = (settings.output == NULL) ? strlen(DEFAULT_OUTPUT) : strlen(settings.output);
    size_t num_chars = len_output + num_digits_in(MAX_NUM_SLICES - 1) + 3;
    size_t num_tris = get_num_tris(&settings);
    size_t *indices = (size_t *) malloc(3 * num_tris * sizeof(size_t));

    if (indices == NULL) {
        fprintf(stderr, "wingstl: error: unable to allocate memory for triangle indices\n");
        free(settings.output);
        free(pts);

        return 1;
    }

    char *full_output = (char *) malloc(num_chars * sizeof(char));

    if (full_output == NULL) {
        fprintf(stderr, "wingstl: error: unable to allocate memory for output file name\n");
        free(settings.output);
        free(indices);
        free(pts);
        return 1;
    }

    for (int islice = 0; islice < settings.num_slices; islice++) {
        assign_indices(&settings, num_tris, islice, indices);
        strcpy(full_output, settings.output == NULL ? DEFAULT_OUTPUT : settings.output);

        if (settings.num_slices > 1) {
            sprintf(full_output + len_output - 4, "(%d).stl", islice);
        }

        if (write_stl(pts, indices, num_tris, full_output)) {
            free(settings.output);
            free(full_output);
            free(indices);
            free(pts);

            return 1;
        }

        if (settings.verbose) {
            printf("(%d/%d) files written successfully\n", islice + 1, settings.num_slices);
        }
    }

    if (settings.verbose) {
        putchar('\n');
        show_settings(&settings);
    }
    
    free(settings.output);
    free(full_output);
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
