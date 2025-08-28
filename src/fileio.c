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

int read_dat(const char *fname, AirfoilData *data) {
    FILE *f = fopen(fname, "r");

    if (f == NULL) {
        fprintf(stderr, "wingstl: error: unable to open .dat file for reading\n");
        return 1;
    }

    int line_no = 1;
    int num_mid_breaks = 0;
    char line[MAX_LINE];
    float x, y;

    LineResult result;
    LineResult last_result;
    bool has_quantity_line = false;
    bool has_break_before_first_point = false;

    data->num_pts = 0;
    while (fgets(line, sizeof(line), f) != NULL) {
        rstrip(line);
        result = parse_line(line, (line_no == 1), &x, &y);

        switch (result) {
            case VALID_HEADER_LINE:
                strcpy(data->header, line);
                break;
            case EMPTY_HEADER_LINE:
                fprintf(stderr, "wingstl: error: .dat file does not contain a header on the first line\n");
                fclose(f);
                return 1;
            case INVALID_FORMAT_LINE:
                fprintf(stderr, "wingstl: error: line %d of .dat file is not formatted correctly\n", line_no);
                fclose(f);
                return 1;
            case EMPTY_BODY_LINE:
                break;
            case VALUE_LINE:
                if (data->num_pts == MAX_AIRFOIL_PTS) {
                    fprintf(stderr, "wingstl: error: .dat file contains too many points\n");
                    fclose(f);
                    return 1;
                }

                data->pts[data->num_pts].x = x;
                data->pts[data->num_pts].y = y;

                num_mid_breaks += (last_result == EMPTY_BODY_LINE && data->num_pts > 0);
                if (num_mid_breaks > 1) {
                    fprintf(stderr, "wingstl: error: redundant middle break on line %d of .dat file\n", line_no - 1);
                    fclose(f);
                    return 1;
                }

                data->num_pts++;
                if (data->num_pts == 1) {
                    has_break_before_first_point = (last_result == EMPTY_BODY_LINE);
                }

                break;
            case POINT_QUANTITY_LINE:
                if (has_quantity_line) {
                    fprintf(stderr, "wingstl: error: redundant quantity of points provided on line %d\n of .dat file", line_no);
                    fclose(f);
                    return 1;
                }

                has_quantity_line = true;
                break;
            default:
                fprintf(stderr, "wingstl: error: unable to parse line %d of .dat file\n", line_no);
                fclose(f);
                return 1;
        }

        line_no++;
        last_result = result;
    }

    if (data->num_pts < MIN_AIRFOIL_PTS) {
        fprintf(stderr, "wingstl: error: .dat file contains less than minimum of %d points\n", MIN_AIRFOIL_PTS);
        fclose(f);
        return 1;
    }

    if (has_quantity_line && num_mid_breaks == 0) {
        fprintf(stderr, "wingstl: error: .dat file contains line for quantity of points but no middle break\n");
        fclose(f);
        return 1;
    }

    if (num_mid_breaks == 1 && !has_break_before_first_point) {
        fprintf(stderr, "wingstl: error: .dat file has middle break but no break before the first point\n");
        fclose(f);
        return 1;
    }

    data->is_lednicer_fmt = (num_mid_breaks > 0);

    fclose(f);
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