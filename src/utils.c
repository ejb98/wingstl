/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

#include "types.h"
#include "utils.h"
#include "constants.h"

float to_radians(float degrees) {
    return degrees * PI_OVER_180;
}

float interp(Vec2D *p0, Vec2D *p2, float x) {
    return p0->y + (p2->y - p0->y) * (x - p0->x) / (p2->x - p0->x); 
}

int num_digits_in(int value) {
    if (value == 0) {
        return 1;
    }

    if (value < 0) {
        value = -value;
    }

    int num_digits = 0;

    while (value > 0) {
        value /= 10;
        num_digits++;
    }

    return num_digits;
}

bool has_ext(char *arg, const char *ext) {
    size_t ext_len = strlen(ext);
    size_t arg_len = strlen(arg);

    char *ptr = strstr(arg, ext);
    return ((ptr != NULL) && (ptr == arg + arg_len - ext_len));
}

void rstrip(char *line) {
    size_t length = strlen(line);

    while (length > 0 && isspace((unsigned char) line[length - 1])) {
        line[length - 1] = '\0';
        length--;
    }
}

bool nearly_equal(float a, float b) {
    return fabs(a - b) < METERS_PER_MICROMETER;
}

Units to_units(const char *str) {
    if (strcmp(str, "m") == 0) {
        return METERS;
    } else if (strcmp(str, "cm") == 0) {
        return CENTIMETERS;
    } else if (strcmp(str, "mm") == 0) {
        return MILLIMETERS;
    } else if (strcmp(str, "ft") == 0) {
        return FEET;
    } else if (strcmp(str, "in") == 0) {
        return INCHES;
    } else {
        return UNKNOWN_UNITS;
    }
}

float to_meters(float value, Units units) {
    switch (units) {
        case FEET:
            return value / FEET_PER_METER;
        case INCHES:
            return value / INCHES_PER_METER;
        case METERS:
            return value;
        case CENTIMETERS:
            return value / 100.0f;
        case MILLIMETERS:
            return value / 1000.0f;
        case UNKNOWN_UNITS:
            fprintf(stderr, "wingstl: warning: value cannot be converted to meters\n");
            return value;
        default:
            fprintf(stderr, "wingstl: warning: value cannot be converted to meters\n");
            return value;
    }
}

size_t sub2ind(int i, int j, int num_cols) {
    return (size_t) i * num_cols + j;
}

void cross(const Vec3D *a, const Vec3D *b, Vec3D *v) {
    v->x = a->y * b->z - a->z * b->y;
    v->y = a->z * b->x - a->x * b->z;
    v->z = a->x * b->y - a->y * b->x;
}

void subtract(const Vec3D *a, const Vec3D *b, Vec3D *v) {
    v->x = a->x - b->x;
    v->y = a->y - b->y;
    v->z = a->z - b->z;
}

void normalize(Vec3D *v) {
    float d = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);

    if (d > METERS_PER_MICROMETER) {
        v->x /= d;
        v->y /= d;
        v->z /= d;
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