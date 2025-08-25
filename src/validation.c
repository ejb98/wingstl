/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#include <stdio.h>

#include "types.h"
#include "engine.h"
#include "constants.h"

void suggest_adjust_values(void) {
    printf("try adjusting values for '%s', '%s', '%s' or '%s'\n", 
            FLAG_SWEEP_LE, FLAG_SWEEP_TE, FLAG_SEMI_SPAN, FLAG_ROOT_CHORD);
}

void suggest_flag_and_value(const char *desc, const char *flag) {
    printf("wingstl: specify %s using the flag '%s' followed by a value\n", desc, flag);
}

int validate_props(const Wing *wing) {
    if (wing->airfoil.m < 0) {
        suggest_flag_and_value("4-digit naca airfoil", FLAG_AIRFOIL);
        return 1;
    }

    if (wing->semi_span < 0.0f) {
        suggest_flag_and_value("semi span", FLAG_SEMI_SPAN);
        return 1;
    }

    if (wing->root_chord < 0.0f) {
        suggest_flag_and_value("root chord", FLAG_ROOT_CHORD);
        return 1;
    }

    if (wing->airfoil.t == 0) {
        printf("wingstl: zero thickness wing detected; ");
        printf("try increasing the third or fourth digit in the value for '%s'", FLAG_AIRFOIL);

        return 1;
    }

    if (tip_overlap(wing)) {
        printf("wingstl: wing tip overlap detected; ");
        suggest_adjust_values();

        return 1;
    }

    float aspect_ratio = get_aspect_ratio(wing);
    if (aspect_ratio < MIN_ASPECT_RATIO || aspect_ratio > MAX_ASPECT_RATIO) {
        printf("wingstl: extreme aspect ratio detected; ");
        suggest_adjust_values();

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