/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "types.h"
#include "engine.h"
#include "constants.h"
#include "validation.h"

void suggest_adjust_values(void) {
    fprintf(stderr, "try adjusting values for '%s', '%s', '%s' or '%s'\n", 
            FLAG_SWEEP_LE, FLAG_SWEEP_TE, FLAG_SEMI_SPAN, FLAG_ROOT_CHORD);
}

void suggest_flag_and_value(const char *desc, const char *flag) {
    fprintf(stderr, "wingstl: error: specify %s using the flag '%s' followed by a value\n", desc, flag);
}

bool x_decreases_then_increases(const Airfoil *airfoil, int istart, int iend) {
    bool was_decreasing = (airfoil->pts[istart + 1].x - airfoil->pts[istart].x) < 0.0f;
    if (!was_decreasing) return false;

    float dx;
    bool switched;
    int num_switches = 0;

    for (int i = istart + 1; i < iend; i++) {
        dx = airfoil->pts[i].x - airfoil->pts[i - 1].x;

        switched = (dx > 0.0f && was_decreasing) || (dx < 0.0f && !was_decreasing);
        num_switches += switched;

        if (num_switches > 1) return false;

        was_decreasing = dx < 0.0f;
    }

    return true;
}

bool x_increases(const Airfoil *airfoil, int istart, int iend) {
    float dx;

    for (int i = istart + 1; i < iend; i++) {
        dx = airfoil->pts[i].x - airfoil->pts[i - 1].x;
        if (dx < 0.0f) return false;
    }

    return true;
}

int validate_airfoil(const Airfoil *airfoil) {
    if (airfoil->num_pts < 0) {
        suggest_flag_and_value("airfoil .dat file or 4-digit naca code", FLAG_AIRFOIL);
        return 1;
    }

    bool is_naca4 = !airfoil->num_pts;

    if (is_naca4 && strlen(airfoil->header) != 4) {
        fprintf(stderr, "wingstl: error: naca airfoil code must be exactly 4 digits\n");
        return 1;
    }

    bool is_all_digits = true;
    for (int i = 0; i < strlen(airfoil->header); i++) {
        is_all_digits = is_all_digits && isdigit((unsigned char) airfoil->header[i]);
    }

    if (is_naca4 && !is_all_digits) {
        fprintf(stderr, "wingstl: error: naca airfoil code must contain only 4 digits\n");
        return 1;
    }

    if (is_naca4 && airfoil->header[2] == '0' && airfoil->header[3] == '0') {
        fprintf(stderr, "wingstl: error: naca airfoil code will result in zero thickness\n");
        return 1;
    }

    if (!is_naca4 && airfoil->num_pts < MIN_AIRFOIL_PTS) {
        fprintf(stderr, "wingstl: error: less than %d points loaded from airfoil .dat file\n", MIN_AIRFOIL_PTS);
        return 1;
    }

    if (!is_naca4 && airfoil->num_pts > MAX_AIRFOIL_PTS) {
        fprintf(stderr, "wingstl: error: more than %d points loaded from airfoil .dat file\n", MAX_AIRFOIL_PTS);
        return 1;
    }

    bool is_selig = !is_naca4 && airfoil->num_pts && !airfoil->lednicer_index;

    if (is_selig && !x_decreases_then_increases(airfoil, 0, airfoil->num_pts)) {
        fprintf(stderr, "wingstl: error: airfoil .dat file points are not ordered correctly\n");
        return 1;
    }

    bool is_lednicer = !is_naca4 && airfoil->num_pts && airfoil->lednicer_index;

    if (is_lednicer && !x_increases(airfoil, 0, airfoil->lednicer_index)) {
        fprintf(stderr, "wingstl: error: airfoil .dat file points are not ordered correctly\n");
        return 1;
    }

    if (is_lednicer && !x_increases(airfoil, airfoil->lednicer_index, airfoil->num_pts)) {
        if (!x_decreases_then_increases(airfoil, airfoil->lednicer_index, airfoil->num_pts)) {
            fprintf(stderr, "wingstl: error: airfoil .dat file points are not ordered correctly\n");
            return 1;
        }
    }

    return 0;
}

int validate_settings(const Settings *settings) {
    if (validate_airfoil(&settings->airfoil)) {
        return 1;
    }

    if (settings->semi_span < 0.0f) {
        suggest_flag_and_value("semi span", FLAG_SEMI_SPAN);
        return 1;
    }

    if (settings->root_chord < 0.0f) {
        suggest_flag_and_value("root chord", FLAG_ROOT_CHORD);
        return 1;
    }

    if (tip_overlap(settings)) {
        fprintf(stderr, "wingstl: error: wing tip overlap detected; ");
        suggest_adjust_values();

        return 1;
    }

    float aspect_ratio = get_aspect_ratio(settings);
    if (aspect_ratio < MIN_ASPECT_RATIO || aspect_ratio > MAX_ASPECT_RATIO) {
        fprintf(stderr, "wingstl: error: extreme aspect ratio detected; ");
        suggest_adjust_values();

        return 1;
    }

    return 0;
}

int validate_file(int num_breaks, int num_quantity, int num_invalid,
                        bool has_break_b4_p0, bool has_empty_header) {
    if (has_empty_header) {
        fprintf(stderr, "wingstl: error: .dat file does not contain a header on the first line\n");
        return 1;
    }

    if (num_invalid > 0) {
        fprintf(stderr, "wingstl: error: line %d of .dat file is not formatted correctly\n", num_invalid);
        return 1;
    }

    if (num_breaks > 1) {
        fprintf(stderr, "wingstl: error: .dat file contains multiple line breaks between points\n");
        return 1;
    }

    if (num_quantity > 1) {
        fprintf(stderr, "wingstl: error: .dat file contains multiple lines with point quantities\n");
        return 1;
    }

    if (num_quantity == 1 && num_breaks == 0) {
        fprintf(stderr, "wingstl: error: .dat file contains a line with point quantities");
        fprintf(stderr, "but does not contains a middle line break between points\n");

        return 1;
    }

    if (num_breaks == 1 && !has_break_b4_p0) {
        fprintf(stderr, "wingstl: error: .dat file contains a line break between points");
        fprintf(stderr, "but does not contains a line break before the first point\n");

        return 1;
    }

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