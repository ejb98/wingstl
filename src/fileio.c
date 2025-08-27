/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "types.h"
#include "constants.h"

void print_file_error(FileError error) {
    switch (error) {
        case NO_ERROR:
            return;
        case WRITE_ERROR:
            fprintf(stderr, "wingstl: error: unable to open file for writing\n");
            return;
        case READ_ERROR:
            fprintf(stderr, "wingstl: error: unable to open file for reading\n");
            return;
        case FORMAT_ERROR:
            fprintf(stderr, "wingstl: error: file is not formatted correctly\n");
            return;
        case EMPTY_ERROR:
            fprintf(stderr, "wingstl: error: file is empty\n");
            return;
        case SIZE_ERROR:
            fprintf(stderr, "wingstl: error: file is too large\n");
            return;
        case UNKNOWN_ERROR:
            fprintf(stderr, "wingstl: error: unable to process file\n");
            return;
        default:
            fprintf(stderr, "wingstl: error: unable to process file\n");
            return;
    }
}

FileError read_dat(const char *fname, Airfoil *airfoil) {
    FILE *fp = fopen(fname, "r");

    if (fp == NULL) {
        return READ_ERROR;
    }

    airfoil->num_pts = 0;
    airfoil->lednicer_index = 0;

    char line[MAX_LINE];

    bool last_line_empty = false;
    bool point_counts_found = false;
    bool empty_line_before_first = false;

    float x;
    float y;

    if (fgets(airfoil->label, sizeof(airfoil->label), fp) != NULL) {
        rstrip(airfoil->label);
    } else {
        fclose(fp);
        return EMPTY_ERROR;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        rstrip(line);

        if (strlen(line) == 0) {
            last_line_empty = true;
            continue;
        }

        if (sscanf(line, "%f %f", &x, &y) == 2) {
            if (x > 2.0f && y > 2.0f && airfoil->num_pts == 0) {
                if (point_counts_found) {
                    fclose(fp);
                    return FORMAT_ERROR;
                }

                point_counts_found = true;
                continue;
            }

            if (airfoil->num_pts < MAX_AIRFOIL_PTS) {
                if (airfoil->num_pts > 1 && last_line_empty) {
                    if (airfoil->lednicer_index > 0 || !empty_line_before_first) {
                        fclose(fp);
                        return FORMAT_ERROR;
                    }

                    airfoil->lednicer_index = airfoil->num_pts;
                }

                airfoil->pts[airfoil->num_pts].x = x;
                airfoil->pts[airfoil->num_pts].y = y;
                airfoil->num_pts++;

                if (airfoil->num_pts == 1 && last_line_empty) {
                    empty_line_before_first = true;
                }

            } else {
                fclose(fp);
                return SIZE_ERROR;
            }

        } else {
            fclose(fp);
            return FORMAT_ERROR;
        }

        last_line_empty = false;
    }

    if (airfoil->num_pts < MIN_AIRFOIL_PTS) {
        fclose(fp);
        return FORMAT_ERROR;
    }

    fclose(fp);
    return NO_ERROR;
}

FileError write_stl(Vec3D *pts, const size_t *indices, size_t num_tris, const char *fname) {
    FILE *fp = fopen(fname, "w");

    if (fp == NULL) {
        return WRITE_ERROR;
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

    return NO_ERROR;
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