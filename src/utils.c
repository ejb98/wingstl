/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#include <math.h>
#include <float.h>
#include <stdbool.h>

#include "types.h"
#include "constants.h"

float to_rads(float degrees) {
    return degrees * PI_OVER_180;
}

float get_aspect_ratio(const wing_props *wing) {
    float b = 2.0f * wing->semi_span;
    float dx_le = wing->semi_span * tanf(to_rads(90.0f - wing->sweep_angles[0]));
    float dx_te = wing->semi_span * tanf(to_rads(90.0f - wing->sweep_angles[1]));
    float s = 2.0f * wing->root_chord * wing->semi_span + wing->semi_span * (dx_te - dx_le);

    return (s > FLT_EPSILON) ? b * b / s : 0.0f;
}

bool tip_overlaps(const wing_props *wing) {
    float offsets[2];

    for (int i = 0; i < 2; i++) {
        offsets[i] = wing->semi_span * (to_rads(90.0f - wing->sweep_angles[i]));
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