/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include "constants.h"

typedef struct Vec2D {
    float x, y;
} Vec2D;

typedef struct Vec3D {
    float x, y, z;
} Vec3D;

typedef enum Units {
    FEET,
    INCHES,
    METERS,
    CENTIMETERS,
    MILLIMETERS,
    UNKNOWN_UNITS
} Units;

typedef enum LineResult {
    VALUE_LINE,
    EMPTY_BODY_LINE,
    EMPTY_HEADER_LINE,
    VALID_HEADER_LINE,
    POINT_QUANTITY_LINE,
    INVALID_FORMAT_LINE
} LineResult;

typedef struct Airfoil {
    int num_pts;
    int lednicer_index;
    char header[MAX_LINE];
    Vec2D pts[MAX_AIRFOIL_PTS];
    bool has_closed_te;
} Airfoil;

typedef struct Settings {
    Units units;
    Airfoil airfoil;

    int num_slices;
    int num_pts_chord;

    float semi_span;
    float root_chord;
    float sweep_angles[2];

    bool help;
    bool verbose;

    char *output;
} Settings;

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