/*
 * Copyright (C) 2025 Ethan Billingsley
 *
 * Licensed under the GNU General Public License v3.0 or later (GPLv3+).
 * See end of file for details.
 */

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <float.h>
#include <ctype.h>
#include <math.h>

#define A0 0.2969f
#define A1 -0.126f
#define A2 -0.3516f
#define A3 0.2843f
#define A4_OPEN -0.1015f
#define A4_CLOSED -0.1036f

#define MAX_SWEEP 90.0f
#define MIN_SWEEP 10.0f

#define PI 3.14159f
#define PI_OVER_180 0.01745f

#define FLAG_AIRFOIL "-a"
#define FLAG_SWEEP_LE "-le"
#define FLAG_SWEEP_TE "-te"
#define FLAG_CHORD_PTS "-p"
#define FLAG_SEMI_SPAN "-b"
#define FLAG_ROOT_CHORD "-c"

#define NUM_SPAN_PTS 2
#define MIN_CHORD_PTS 20
#define MAX_CHORD_PTS 200

#define HAS_CLOSED_TE 1
#define HAS_COSINE_SPACING 1

typedef struct vec3 {
    float x, y, z;
} vec3;

typedef struct naca4 {
    int m, p, t;
} naca4;

typedef struct wing_props {
    naca4 airfoil;

    int num_pts_span;
    int num_pts_chord;

    float semi_span;
    float root_chord;
    float sweep_angles[2];

    bool has_closed_te;
    bool has_cosine_spacing;
} wing_props;

size_t sub2ind(int i, int j, int num_cols) {
    return (size_t) i * num_cols + j;
}

float get_camber(float x, float m, float p) {
    float a = 2.0f * p * x - x * x;

    if (x < p && p > FLT_EPSILON) {
        return m * a / (p * p);
    }

    float b = 1.0f - p;

    return m * (1.0f - 2.0f * p + a) / (b * b);
}

float get_gradient(float x, float m, float p) {
    float a = (2.0f * m) * (p - x);

    if (x < p && p > FLT_EPSILON) {
        return a / (p * p);
    }

    float b = 1.0f - p;

    return a / (b * b);
}

float get_thickness(float x, float t, bool is_closed) {
    float x2 = x * x;
    float a4 = is_closed ? A4_CLOSED: A4_OPEN;  

    return (A0 * powf(x, 0.5f) + A1 * x + A2 * x2 + 
            A3 * x2 * x + a4 * x2 * x2) * t / 0.2f;
}

float get_x_surf(float xc, float thickness, float theta, bool is_upper) {
    float sign = is_upper ? -1.0f : 1.0f;

    return xc + sign * thickness * sinf(theta);
}

float get_z_surf(float zc, float thickness, float theta, bool is_upper) {
    float sign = is_upper ? 1.0f : -1.0f;

    return zc + sign * thickness * cosf(theta);
}

void cross(const vec3 *a, const vec3 *b, vec3 *v) {
    v->x = a->y * b->z - a->z * b->y;
    v->y = a->z * b->x - a->x * b->z;
    v->z = a->x * b->y - a->y * b->x;
}

void subtract(const vec3 *a, const vec3 *b, vec3 *v) {
    v->x = a->x - b->x;
    v->y = a->y - b->y;
    v->z = a->z - b->z;
}

void normalize(vec3 *v) {
    float d = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);

    if (d <= FLT_EPSILON) {
        return;
    }

    v->x /= d;
    v->y /= d;
    v->z /= d;
}

float to_rads(float degrees) {
    return degrees * PI_OVER_180;
}

size_t get_num_pts(const wing_props *wing) {
    return (size_t) wing->num_pts_span * (2 * wing->num_pts_chord - wing->has_closed_te - 1);
}

size_t get_num_tris(const wing_props *wing) {
    size_t num_tris_surf = (wing->num_pts_chord - 1) * (wing->num_pts_span - 1) * 2;
    size_t num_tris_side = 2 * wing->num_pts_chord - wing->has_closed_te - 3;
    size_t num_tris_aft = !wing->has_closed_te * (wing->num_pts_span - 1) * 2;
    
    return 2 * (num_tris_surf + num_tris_side) + num_tris_aft;
}

vec3 *make_pts(const wing_props *wing) {
    vec3 *pts = (vec3 *) malloc(get_num_pts(wing) * sizeof(vec3));

    if (pts == NULL) {
        return NULL;
    }
    
    float dx_le;
    float theta;
    float xn_surf;
    float zn_surf;
    float y_camber;
    float xn_camber;
    float zn_camber;
    float thickness;
    float local_chord;

    int row_max = wing->num_pts_chord;
    int num_rows = wing->num_pts_chord;
    int num_cols = wing->num_pts_span;
    int row_start = 0;

    float m = wing->airfoil.m / 100.0f;
    float t = wing->airfoil.t / 100.0f;
    float p = wing->airfoil.p / 10.0f;

    float tan_le = tanf(to_rads(90.0f - wing->sweep_angles[0]));
    float tan_te = tanf(to_rads(90.0f - wing->sweep_angles[1]));
    float tan_diff = tan_te - tan_le;

    size_t ind;

    for (int is_upper = 1; is_upper >= 0; is_upper--) {
        if (!is_upper) {
            row_start = 1;
            row_max = num_rows - wing->has_closed_te;
        }

        for (int j = 0; j < num_cols; j++) {
            dx_le = y_camber * tan_le;
            y_camber = wing->semi_span * j / (num_cols - 1);
            local_chord = wing->root_chord + y_camber * tan_diff;

            for (int i = row_start; i < row_max; i++) {
                if (is_upper) {
                    ind = sub2ind(i - row_start, j, num_cols);
                } else {
                    ind = (size_t) num_rows * num_cols + sub2ind(i - row_start, j, num_cols);
                }

                xn_camber = (float) i / (num_rows - 1);

                if (wing->has_cosine_spacing) {
                    xn_camber = (1.0f - cosf(xn_camber * PI)) / 2.0f;
                }

                theta = get_gradient(xn_camber, m, p);
                thickness = get_thickness(xn_camber, t, wing->has_closed_te);
                zn_camber = get_camber(xn_camber, m, p);
                zn_surf = get_z_surf(zn_camber, thickness, theta, is_upper);
                xn_surf = get_x_surf(xn_camber, thickness, theta, is_upper);

                pts[ind].y = y_camber;
                pts[ind].z = zn_surf * local_chord;
                pts[ind].x = xn_surf * local_chord + dx_le;
            }
        }
    }

    return pts;
}

size_t get_upper_ind(const wing_props *wing, int i, int j) {
    return sub2ind(i, j, wing->num_pts_span);
}

size_t get_lower_ind(const wing_props *wing, int i, int j) {
    bool is_last_row = (i == wing->num_pts_chord - 1);

    if (i == 0 || (is_last_row && wing->has_closed_te)) {
        return sub2ind(i, j, wing->num_pts_span);
    }

    size_t offset = sub2ind(i - 1, j, wing->num_pts_span);
    return (size_t) wing->num_pts_chord * wing->num_pts_span + offset;
}

size_t fill_upper_lower_inds(const wing_props *wing, size_t k, size_t *inds) {
    size_t corners[4];

    for (int is_upper = 1; is_upper >= 0; is_upper--) {
        for (int i = 0; i < wing->num_pts_chord - 1; i++) {
            for (int j = 0; j < wing->num_pts_span - 1; j++) {
                if (is_upper) {
                    corners[0] = get_upper_ind(wing, i, j);
                    corners[1] = get_upper_ind(wing, i, j + 1);
                    corners[2] = get_upper_ind(wing, i + 1, j + 1);
                    corners[3] = get_upper_ind(wing, i + 1, j);
                } else {
                    corners[0] = get_lower_ind(wing, i, j + 1);
                    corners[1] = get_lower_ind(wing, i, j);
                    corners[2] = get_lower_ind(wing, i + 1, j);
                    corners[3] = get_lower_ind(wing, i + 1, j + 1);
                }

                inds[k++] = corners[3];
                inds[k++] = corners[2];
                inds[k++] = corners[1];
                inds[k++] = corners[3];
                inds[k++] = corners[1];
                inds[k++] = corners[0];
            }
        }
    }

    return k;
}

size_t fill_port_star_inds(const wing_props *wing, size_t k, size_t *inds) {
    int j;
    bool is_last_row;
    size_t corners[4];

    for (int is_port = 1; is_port >= 0; is_port--) { 
        j = !is_port * (wing->num_pts_span - 1);

        for (int i = 0; i < wing->num_pts_chord - 1; i++) {
            is_last_row = (i == wing->num_pts_chord - 2);

            if (i == 0) {
                inds[k++] = get_upper_ind(wing, i, j);

                if (is_port) {
                    inds[k++] = get_lower_ind(wing, i + 1, j);
                    inds[k++] = get_upper_ind(wing, i + 1, j);
                } else {
                    inds[k++] = get_upper_ind(wing, i + 1, j);
                    inds[k++] = get_lower_ind(wing, i + 1, j);
                }
            } else if (is_last_row && wing->has_closed_te) {
                inds[k++] = get_lower_ind(wing, i + 1, j);

                if (is_port) {
                    inds[k++] = get_upper_ind(wing, i, j);
                    inds[k++] = get_lower_ind(wing, i, j);
                } else {
                    inds[k++] = get_lower_ind(wing, i, j);
                    inds[k++] = get_upper_ind(wing, i, j);
                }
            } else {
                if (is_port) {
                    corners[0] = get_lower_ind(wing, i, j);
                    corners[1] = get_lower_ind(wing, i + 1, j);
                    corners[2] = get_upper_ind(wing, i + 1, j);
                    corners[3] = get_upper_ind(wing, i, j);
                } else {
                    corners[0] = get_lower_ind(wing, i + 1, j);
                    corners[1] = get_lower_ind(wing, i, j);
                    corners[2] = get_upper_ind(wing, i, j);
                    corners[3] = get_upper_ind(wing, i + 1, j);
                }

                inds[k++] = corners[0];
                inds[k++] = corners[1];
                inds[k++] = corners[2];
                inds[k++] = corners[0];
                inds[k++] = corners[2];
                inds[k++] = corners[3];
            }
        }
    }

    return k;
}

size_t fill_aft_inds(const wing_props *wing, size_t k, size_t *inds) {
    int i = wing->num_pts_chord - 1;
    size_t corners[4];

    for (int j = 0; j < wing->num_pts_span - 1; j++) {
        corners[0] = get_lower_ind(wing, i, j);
        corners[1] = get_lower_ind(wing, i, j + 1);
        corners[2] = get_upper_ind(wing, i, j + 1);
        corners[3] = get_upper_ind(wing, i, j);

        inds[k++] = corners[0];
        inds[k++] = corners[1];
        inds[k++] = corners[2];
        inds[k++] = corners[0];
        inds[k++] = corners[2];
        inds[k++] = corners[3];
    }

    return k;
}

size_t *make_inds(const wing_props *wing) {
    size_t k = 0;
    size_t num_tris = get_num_tris(wing);
    size_t *inds = (size_t *) malloc(3 * num_tris * sizeof(size_t));

    if (inds == NULL) {
        return NULL;
    }

    k = fill_upper_lower_inds(wing, k, inds);
    k = fill_port_star_inds(wing, k, inds);

    if (!wing->has_closed_te) {
        k = fill_aft_inds(wing, k, inds);
    }

    size_t num_tris_created = k / 3;
    assert(num_tris_created == num_tris);

    return inds;
}

void write_stl(vec3 *pts, const size_t *indices, size_t num_tris, const char *file_name) {
    FILE *fp = fopen(file_name, "w");

    if (fp == NULL) {
        fprintf(stderr, "wingstl: error: unable to open STL file\n");

        return;
    }

    size_t k = 0;

    vec3 a, b, n;
    vec3 *v0 = NULL;
    vec3 *v1 = NULL;
    vec3 *v2 = NULL;

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
}

float handle_semi_span(int iarg, int num_args, char **args) {
    float semi_span = -1.0f;

    if (iarg + 1 < num_args) {
        semi_span = atof(args[iarg + 1]);

        if (semi_span <= 0.0f) {
            fprintf(stderr, "wingstl: error: value for option %s must be a nonzero positive number (e.g., 6.0)\n", FLAG_SEMI_SPAN);

            return -1.0f;
        }

    } else {
        fprintf(stderr, "wingstl: error: option %s (semi span) requires a value (e.g., 6.0)\n", FLAG_SEMI_SPAN);

        return -1.0f;
    }

    return semi_span;
}

float handle_root_chord(int iarg, int num_args, char **args) {
    float root_chord = -1.0f;

    if (iarg + 1 < num_args) {
        root_chord = atof(args[iarg + 1]);

        if (root_chord <= 0.0f) {
            fprintf(stderr, "wingstl: error: value for option %s must be a nonzero positive number (e.g., 1.0)\n", FLAG_ROOT_CHORD);

            return -1.0f;
        }

    } else {
        fprintf(stderr, "wingstl: error: option %s (root chord) requires a value (e.g., 1.0)\n", FLAG_ROOT_CHORD);

        return -1.0f;
    }

    return root_chord;
}

naca4 handle_airfoil(int iarg, int num_args, char **args) {
    naca4 airfoil = {.m = -1};

    if (iarg + 1 < num_args) {
        char *arg = args[iarg + 1];
        int num_digits = strlen(arg);

        if (num_digits != 4) {
            fprintf(stderr, "wingstl: error: value for option %s must be exactly four digits (e.g., 2412)\n", FLAG_AIRFOIL);

            return airfoil;
        }

        char digit;
        for (int j = 0; j < num_digits; j++) {
            digit = arg[j];

            if (!isdigit(digit)) {
                fprintf(stderr, "wingstl: error: value for option %s must contain only digits (e.g., 2412)\n", FLAG_AIRFOIL);

                airfoil.m = -1;
                return airfoil;
            }

            switch (j) {
                case 0:
                    airfoil.m = digit - '0';
                    break;
                case 1:
                    airfoil.p = digit - '0';
                    break;
                case 2:
                    airfoil.t = digit - '0';
                    break;
                case 3:
                    airfoil.t = 10 * airfoil.t + (digit - '0');
                    break;
                default:
                    fprintf(stderr, "wingstl: error: value for option %s must be exactly four digits (e.g., 2412)\n", FLAG_AIRFOIL);

                    airfoil.m = -1;
                    return airfoil;
            }
        }

    } else {
        fprintf(stderr, "wingstl: error: option %s (naca 4-digit airfoil) requires a value (e.g., 2412)\n", FLAG_AIRFOIL);

        return airfoil;
    }

    return airfoil;
}

int handle_chord_pts(int iarg, int num_args, char **args) {
    int num_pts;

    if (iarg + 1 < num_args) {
        char *arg = args[iarg + 1];

        if (strchr(arg, '.')) {
            fprintf(stderr, "wingstl: error: value for option %s must be an integer\n", FLAG_CHORD_PTS);

            return -1;
        }

        num_pts = atoi(arg);

        if (num_pts < MIN_CHORD_PTS) {
            fprintf(stderr, "wingstl: error: value for option %s must be %d at least\n", FLAG_CHORD_PTS, MIN_CHORD_PTS);

            return -1;
        }

        if (num_pts > MAX_CHORD_PTS) {
            fprintf(stderr, "wingstl: error: value for option %s must be %d at most\n", FLAG_CHORD_PTS, MAX_CHORD_PTS);

            return -1;
        }

    } else {
        fprintf(stderr, "wingstl: error: option %s (number of chordwise points) requires a value (e.g., 50)\n", FLAG_CHORD_PTS);

        return -1;
    }

    return num_pts;
}

float handle_sweep(int iarg, int num_args, char **args, const char *arg_flag) {
    float sweep = -1.0f;

    if (iarg + 1 < num_args) {
        char *arg = args[iarg + 1];
        sweep = atof(arg);

        if (sweep < MIN_SWEEP) {
            fprintf(stderr, "wingstl: error: value for option %s must be %f at least\n", arg_flag , MIN_SWEEP);

            return -1.0f;
        }

        if (sweep > MAX_SWEEP) {
            fprintf(stderr, "wingstl: error: value for option %s must be %f at most\n", arg_flag, MAX_SWEEP);

            return -1.0f;
        }

    } else {
        fprintf(stderr, "wingstl: error: option %s (leading edge sweep angle) requires a value (e.g., 80.0)\n", arg_flag);

        return -1.0f;
    }

    return sweep;
}

bool wing_tip_overlaps(const wing_props *wing) {
    float offsets[2];

    for (int i = 0; i < 2; i++) {
        offsets[i] = wing->semi_span * (to_rads(90.0f - wing->sweep_angles[i]));
    }

    return wing->root_chord + offsets[1] <= offsets[0];
}

int handle_inputs(int num_args, char **args, wing_props *wing) {
    char *arg = NULL;

    for (int i = 1; i < num_args; i++) {
        arg = args[i];

        if (arg[0] != '-') {
            fprintf(stderr, "wingstl: error: argument flags must begin with a hyphen '-'\n");

            return 1;
        }

        if (strcmp(arg, FLAG_SEMI_SPAN) == 0) {
            wing->semi_span = handle_semi_span(i, num_args, args);
            i++;

            if (wing->semi_span < 0.0f) {
                return 1;
            }

        } else if (strcmp(arg, FLAG_ROOT_CHORD) == 0) {
            wing->root_chord = handle_root_chord(i, num_args, args);
            i++;

            if (wing->root_chord < 0.0f) {
                return 1;
            }

        } else if (strcmp(arg, FLAG_AIRFOIL) == 0) {
            wing->airfoil = handle_airfoil(i, num_args, args);
            i++;

            if (wing->airfoil.m < 0) {
                return 1;
            }
        } else if (strcmp(arg, FLAG_CHORD_PTS) == 0) {
            wing->num_pts_chord = handle_chord_pts(i, num_args, args);
            i++;

            if (wing->num_pts_chord < 0) {
                return 1;
            }

        } else if (strcmp(arg, FLAG_SWEEP_LE) == 0) {
            wing->sweep_angles[0] = handle_sweep(i, num_args, args, FLAG_SWEEP_LE);
            i++;

            if (wing->sweep_angles[0] < 0.0f) {
                return 1;
            }

        } else if (strcmp(arg, FLAG_SWEEP_TE) == 0) {
            wing->sweep_angles[0] = handle_sweep(i, num_args, args, FLAG_SWEEP_TE);
            i++;

            if (wing->sweep_angles[0] < 0.0f) {
                return 1;
            }

        } else {
            fprintf(stderr, "wingstl: error: unrecognized argument flag '%s'\n", arg);

            return 1;
        }
    }

    return 0;
}

int main(int argc, char **argv) {
    wing_props wing = {
        .airfoil = {.m = -1},
        .semi_span = -1.0f,
        .root_chord = -1.0f,
        .sweep_angles = {MAX_SWEEP, MAX_SWEEP},
        .num_pts_span = NUM_SPAN_PTS,
        .num_pts_chord = MIN_CHORD_PTS,
        .has_closed_te = HAS_CLOSED_TE,
        .has_cosine_spacing = HAS_COSINE_SPACING
    };

    if (handle_inputs(argc, argv, &wing)) {
        return 1;
    }

    if (wing.airfoil.m < 0) {
        fprintf(stderr, "wingstl: error: please provide a four-digit number for the naca airfoil using flag '%s'\n", FLAG_AIRFOIL);

        return 1;
    }

    if (wing.semi_span < 0.0f) {
        fprintf(stderr, "wingstl: error: please provide a value for the semi span using flag '%s'\n", FLAG_SEMI_SPAN);
        
        return 1;
    }

    if (wing.root_chord < 0.0f) {
        fprintf(stderr, "wingstl: error: please provide a value for the root chord using flag '%s'\n", FLAG_ROOT_CHORD);
        
        return 1;
    }

    if (wing_tip_overlaps(&wing)) {
        fprintf(stderr, "wingstl: error: wing tip overlap detected\n");
        fprintf(stderr, "         try using a different value for '%s', '%s', or '%s'\n", 
                                  FLAG_SWEEP_LE, FLAG_SWEEP_TE, FLAG_SEMI_SPAN);

        return 1;
    }

    vec3 *pts = make_pts(&wing);

    if (pts == NULL) {
        fprintf(stderr, "wingstl: error: unable to allocate memory for surface vertices\n");

        return 1;
    }

    size_t *indices = make_inds(&wing);

    if (indices == NULL) {
        fprintf(stderr, "wingstl: error: unable to allocate memory for triangle indices\n");
        free(pts);

        return 1;
    }

    write_stl(pts, indices, get_num_tris(&wing), "wing.stl");
    
    free(indices);
    free(pts);

    return 0;
}

/*
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
 */