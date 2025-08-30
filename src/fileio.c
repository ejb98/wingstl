/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "utils.h"
#include "types.h"
#include "engine.h"
#include "validation.h"

int write_stl(Vec3D *pts, const size_t *indices, size_t num_tris, const char *fname) {
    FILE *fp = fopen(fname, "w");

    if (fp == NULL) {
        fprintf(stderr, "wingstl: error: unable to open .stl file for writing\n");
        return 1;
    }

    size_t k = 0;

    Vec3D a, b, n;
    Vec3D *v0 = NULL;
    Vec3D *v1 = NULL;
    Vec3D *v2 = NULL;

    fprintf(fp, "solid \n");

    for (size_t i = 0; i < num_tris; i++) {
        v0 = pts + indices[k++];
        v1 = pts + indices[k++];
        v2 = pts + indices[k++];

        subtract(v1, v0, &a);
        subtract(v2, v0, &b);
        cross(&a, &b, &n);
        normalize(&n);

        fprintf(fp, "  facet normal %f %f %f\n", n.x, n.y, n.z);
        fprintf(fp, "    outer loop\n");
        fprintf(fp, "      vertex %f %f %f\n", v0->x, v0->y, v0->z);
        fprintf(fp, "      vertex %f %f %f\n", v1->x, v1->y, v1->z);
        fprintf(fp, "      vertex %f %f %f\n", v2->x, v2->y, v2->z);
        fprintf(fp, "    endloop\n");
        fprintf(fp, "  endfacet\n");
    }

    fprintf(fp, "endsolid ");
    fclose(fp);

    return 0;
}

LineResult parse_line(const char *line, bool first_line, float *x, float *y) {
    if (first_line) {
        return !strlen(line) ? EMPTY_HEADER_LINE : VALID_HEADER_LINE;
    }

    if (!strlen(line)) {
        return EMPTY_BODY_LINE;
    }

    int num_values = sscanf(line, "%f %f", x, y);
    if (num_values != 2) {
        return INVALID_FORMAT_LINE;
    }

    if (*x > 1.0f && *y > 1.0f) {
        return POINT_QUANTITY_LINE;
    }

    return VALUE_LINE;
}

int read_dat(const char *fname, Airfoil *airfoil) {
    FILE *f = fopen(fname, "r");

    if (f == NULL) {
        fprintf(stderr, "wingstl: error: unable to open .dat file for reading\n");
        return 1;
    }

    int line_no = 1;
    int num_invalid = 0;
    int num_breaks = 0;
    int num_quantity = 0;

    float xmin, xmax, x, y;
    char line[MAX_LINE];

    LineResult result, last_result;

    bool has_break_b4_p0 = false;
    bool has_empty_header = false;

    airfoil->num_pts = 0;

    while (fgets(line, sizeof(line), f) != NULL) {
        rstrip(line);
        result = parse_line(line, (line_no == 1), &x, &y);

        switch (result) {
            case VALID_HEADER_LINE:
                strcpy(airfoil->header, line);
                break;
            case EMPTY_HEADER_LINE:
                has_empty_header = true;
                break;
            case INVALID_FORMAT_LINE:
                num_invalid = line_no;
                break;
            case EMPTY_BODY_LINE:
                break;
            case VALUE_LINE:
                if (airfoil->num_pts == MAX_AIRFOIL_PTS) {
                    fprintf(stderr, "wingstl: error: .dat file contains too many points\n");
                    fclose(f); return 1;
                }

                airfoil->pts[airfoil->num_pts].x = x;
                airfoil->pts[airfoil->num_pts].y = y;
                if (last_result == EMPTY_BODY_LINE && airfoil->num_pts > 0) {
                    num_breaks += 1;
                    airfoil->lednicer_index = airfoil->num_pts;
                }

                airfoil->num_pts++;
                if (airfoil->num_pts == 1) {
                    xmin = x;
                    xmax = x;
                    has_break_b4_p0 = (last_result == EMPTY_BODY_LINE);
                } else {
                    xmin = (x < xmin) ? x : xmin;
                    xmax = (x > xmax) ? x : xmax;
                }

                break;
            case POINT_QUANTITY_LINE:
                num_quantity += 1;
                break;
            default:
                fprintf(stderr, "wingstl: error: unable to parse line %d of .dat file\n", line_no);
                fclose(f); return 1;
        }

        line_no++;
        last_result = result;
    }

    if (validate_file(num_breaks, num_quantity, num_invalid, has_break_b4_p0, has_empty_header)) {
        fclose(f); return 1;
    }

    adjust_and_scale(airfoil, xmin, xmax);
    fclose(f); return 0;
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