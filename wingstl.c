// Copyright (C) 2025 Ethan Billingsley

// Licensed under the GNU General Public License v3.0 or later (GPLv3+).
// See end of file for details.

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <float.h>
#include <math.h>

#define M 0
#define P 0
#define T 12
#define PI 3.14159f
#define A0 0.2969f
#define A1 -0.126f
#define A2 -0.3516f
#define A3 0.2843f
#define A4_OPEN -0.1015f
#define A4_CLOSED -0.1036f
#define SEMI_SPAN 6.0f
#define ROOT_CHORD 1.0f
#define PI_OVER_180 0.01745f
#define LEADING_SWEEP 80.0f
#define TRAILING_SWEEP 85.0f
#define USE_COSINE_SPACING 1
#define CLOSE_TRAILING_EDGE 0
#define NUM_PANELS_SPANWISE 1
#define NUM_PANELS_CHORDWISE 19

typedef struct Vector3D {
    float x, y, z;
} Vector3D;

typedef struct NACA4Digit {
    int m, p, t;
} NACA4Digit;

typedef struct Wing {
    NACA4Digit airfoil;

    int num_points_spanwise;
    int num_points_chordwise;

    float semi_span;
    float root_chord;
    float sweep_angle_leading;
    float sweep_angle_trailing;

    bool has_cosine_chord_spacing;
    bool has_closed_trailing_edge;
} Wing;

size_t sub2ind(int i, int j, int num_columns) {
    return (size_t) i * num_columns + j;
}

float compute_camber(float x, float m, float p) {
    float a = 2.0f * p * x - x * x;
    if (x < p && p > FLT_EPSILON) return m * a / (p * p);

    float b = 1.0f - p;
    return m * (1.0f - 2.0f * p + a) / (b * b);
}

float compute_gradient(float x, float m, float p) {
    float a = (2.0f * m) * (p - x);
    if (x < p && p > FLT_EPSILON) return a / (p * p);

    float b = 1.0f - p;
    return a / (b * b);
}

float compute_thickness(float x, float t, bool is_closed) {
    float x2 = x * x;
    float a4 = is_closed ? A4_CLOSED: A4_OPEN;      
    return (A0 * powf(x, 0.5f) + A1 * x + A2 * x2 + 
            A3 * x2 * x + a4 * x2 * x2) * t / 0.2f;
}

float compute_surface_x(float xc, float thickness, float theta, bool is_upper) {
    float sign = is_upper ? -1.0f : 1.0f;
    return xc + sign * thickness * sinf(theta);
}

float compute_surface_z(float zc, float thickness, float theta, bool is_upper) {
    float sign = is_upper ? 1.0f : -1.0f;
    return zc + sign * thickness * cosf(theta);
}

void cross(const Vector3D *a, const Vector3D *b, Vector3D *v) {
    v->x = a->y * b->z - a->z * b->y;
    v->y = a->z * b->x - a->x * b->z;
    v->z = a->x * b->y - a->y * b->x;
}

void subtract(const Vector3D *a, const Vector3D *b, Vector3D *v) {
    v->x = a->x - b->x;
    v->y = a->y - b->y;
    v->z = a->z - b->z;
}

void normalize(Vector3D *v) {
    float d = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);

    if (d <= FLT_EPSILON) return;

    v->x /= d;
    v->y /= d;
    v->z /= d;
}

float to_radians(float degrees) {
    return degrees * PI_OVER_180;
}

size_t compute_num_points(const Wing *wing) {
    return (size_t) wing->num_points_spanwise * (2 *
                    wing->num_points_chordwise - 
                    wing->has_closed_trailing_edge - 1);
}

size_t compute_num_triangles(const Wing *wing) {
    size_t num_rows = wing->num_points_chordwise;
    size_t num_columns = wing->num_points_spanwise;
    size_t num_tris_surface = (num_rows - 1) * (num_columns - 1) * 2;
    size_t num_tris_side = 2 * num_rows - wing->has_closed_trailing_edge - 3;
    size_t num_tris_aft = !wing->has_closed_trailing_edge * (num_columns - 1) * 2;
    
    return 2 * (num_tris_surface + num_tris_side) + num_tris_aft;
}

Vector3D *create_points(const Wing *wing) {
    size_t num_points = compute_num_points(wing);
    Vector3D *points = (Vector3D *) malloc(num_points * sizeof(Vector3D));

    if (points == NULL) return NULL;
    
    float theta;
    float y_camber;
    float xn_camber;
    float zn_camber;
    float thickness;
    float xn_surface;
    float zn_surface;
    float dx_leading;
    float local_chord;

    int row_max = wing->num_points_chordwise;
    int num_rows = wing->num_points_chordwise;
    int row_start = 0;
    int num_columns = wing->num_points_spanwise;

    float m = wing->airfoil.m / 100.0f;
    float t = wing->airfoil.t / 100.0f;
    float p = wing->airfoil.p / 10.0f;
    float tan_leading = tanf(to_radians(90.0f - wing->sweep_angle_leading));
    float tan_trailing = tanf(to_radians(90.0f - wing->sweep_angle_trailing));
    float tan_difference = tan_trailing - tan_leading;

    size_t index;

    for (int is_upper = 1; is_upper >= 0; is_upper--) {
        if (!is_upper) {
            row_start = 1;
            row_max = num_rows - wing->has_closed_trailing_edge;
        }

        for (int j = 0; j < num_columns; j++) {
            y_camber = wing->semi_span * j / (num_columns - 1);
            dx_leading = y_camber * tan_leading;
            local_chord = wing->root_chord + y_camber * tan_difference;

            for (int i = row_start; i < row_max; i++) {
                if (is_upper) {
                    index = sub2ind(i - row_start, j, num_columns);
                } else {
                    index = (size_t) num_rows * num_columns + sub2ind(i - row_start, j, num_columns);
                }

                xn_camber = (float) i / (num_rows - 1);

                if (wing->has_cosine_chord_spacing) {
                    xn_camber = (1.0f - cosf(xn_camber * PI)) / 2.0f;
                }

                theta = compute_gradient(xn_camber, m, p);
                thickness = compute_thickness(xn_camber, t, wing->has_closed_trailing_edge);
                zn_camber = compute_camber(xn_camber, m, p);
                zn_surface = compute_surface_z(zn_camber, thickness, theta, is_upper);
                xn_surface = compute_surface_x(xn_camber, thickness, theta, is_upper);

                points[index].y = y_camber;
                points[index].z = zn_surface * local_chord;
                points[index].x = xn_surface * local_chord + dx_leading;
            }
        }
    }

    return points;
}

size_t get_upper_surface_index(const Wing *wing, int i, int j) {
    return sub2ind(i, j, wing->num_points_spanwise);
}

size_t get_lower_surface_index(const Wing *wing, int i, int j) {
    bool is_last_row = (i == wing->num_points_chordwise - 1);

    if (i == 0 || (is_last_row && wing->has_closed_trailing_edge)) {
        return sub2ind(i, j, wing->num_points_spanwise);
    }

    size_t offset = sub2ind(i - 1, j, wing->num_points_spanwise);
    return (size_t) wing->num_points_chordwise * wing->num_points_spanwise + offset;
}

size_t assign_upper_lower_indices(const Wing *wing, size_t k, size_t *indices) {
    size_t corners[4];

    for (int is_upper = 1; is_upper >= 0; is_upper--) {
        for (int i = 0; i < wing->num_points_chordwise - 1; i++) {
            for (int j = 0; j < wing->num_points_spanwise - 1; j++) {
                if (is_upper) {
                    corners[0] = get_upper_surface_index(wing, i, j);
                    corners[1] = get_upper_surface_index(wing, i, j + 1);
                    corners[2] = get_upper_surface_index(wing, i + 1, j + 1);
                    corners[3] = get_upper_surface_index(wing, i + 1, j);
                } else {
                    corners[0] = get_lower_surface_index(wing, i, j + 1);
                    corners[1] = get_lower_surface_index(wing, i, j);
                    corners[2] = get_lower_surface_index(wing, i + 1, j);
                    corners[3] = get_lower_surface_index(wing, i + 1, j + 1);
                }

                indices[k++] = corners[3];
                indices[k++] = corners[2];
                indices[k++] = corners[1];
                indices[k++] = corners[3];
                indices[k++] = corners[1];
                indices[k++] = corners[0];
            }
        }
    }

    return k;
}

size_t assign_port_starboard_indices(const Wing *wing, size_t k, size_t *indices) {
    int j;
    bool is_last_row;
    size_t corners[4];

    for (int is_port = 1; is_port >= 0; is_port--) { 
        j = !is_port * (wing->num_points_spanwise - 1);

        for (int i = 0; i < wing->num_points_chordwise - 1; i++) {
            is_last_row = (i == wing->num_points_chordwise - 2);

            if (i == 0) {
                indices[k++] = get_upper_surface_index(wing, i, j);

                if (is_port) {
                    indices[k++] = get_lower_surface_index(wing, i + 1, j);
                    indices[k++] = get_upper_surface_index(wing, i + 1, j);
                } else {
                    indices[k++] = get_upper_surface_index(wing, i + 1, j);
                    indices[k++] = get_lower_surface_index(wing, i + 1, j);
                }
            } else if (is_last_row && wing->has_closed_trailing_edge) {
                indices[k++] = get_lower_surface_index(wing, i + 1, j);

                if (is_port) {
                    indices[k++] = get_upper_surface_index(wing, i, j);
                    indices[k++] = get_lower_surface_index(wing, i, j);
                } else {
                    indices[k++] = get_lower_surface_index(wing, i, j);
                    indices[k++] = get_upper_surface_index(wing, i, j);
                }
            } else {
                if (is_port) {
                    corners[0] = get_lower_surface_index(wing, i, j);
                    corners[1] = get_lower_surface_index(wing, i + 1, j);
                    corners[2] = get_upper_surface_index(wing, i + 1, j);
                    corners[3] = get_upper_surface_index(wing, i, j);
                } else {
                    corners[0] = get_lower_surface_index(wing, i + 1, j);
                    corners[1] = get_lower_surface_index(wing, i, j);
                    corners[2] = get_upper_surface_index(wing, i, j);
                    corners[3] = get_upper_surface_index(wing, i + 1, j);
                }

                indices[k++] = corners[0];
                indices[k++] = corners[1];
                indices[k++] = corners[2];
                indices[k++] = corners[0];
                indices[k++] = corners[2];
                indices[k++] = corners[3];
            }
        }
    }

    return k;
}

size_t assign_aft_indices(const Wing *wing, size_t k, size_t *indices) {
    int i = wing->num_points_chordwise - 1;
    size_t corners[4];

    for (int j = 0; j < wing->num_points_spanwise - 1; j++) {
        corners[0] = get_lower_surface_index(wing, i, j);
        corners[1] = get_lower_surface_index(wing, i, j + 1);
        corners[2] = get_upper_surface_index(wing, i, j + 1);
        corners[3] = get_upper_surface_index(wing, i, j);

        indices[k++] = corners[0];
        indices[k++] = corners[1];
        indices[k++] = corners[2];
        indices[k++] = corners[0];
        indices[k++] = corners[2];
        indices[k++] = corners[3];
    }

    return k;
}

size_t *create_indices(const Wing *wing) {
    size_t k = 0;
    size_t num_triangles = compute_num_triangles(wing);
    size_t *indices = (size_t *) malloc(3 * num_triangles * sizeof(size_t));

    if (indices == NULL) {
        return NULL;
    }

    k = assign_upper_lower_indices(wing, k, indices);
    k = assign_port_starboard_indices(wing, k, indices);

    if (!wing->has_closed_trailing_edge) {
        k = assign_aft_indices(wing, k, indices);
    }

    size_t num_triangles_created = k / 3;
    assert(num_triangles_created == num_triangles);

    return indices;
}

void write_to_stl(Vector3D *points, const size_t *indices, size_t num_triangles, const char *file_name) {
    FILE *fp = fopen(file_name, "w");

    if (fp == NULL) {
        fprintf(stderr, "wingstl: error: unable to open STL file\n");

        return;
    }

    size_t k = 0;

    Vector3D a, b, n;
    Vector3D *v0, *v1, *v2;

    fprintf(fp, "solid \n");

    for (size_t i = 0; i < num_triangles; i++) {
        v0 = points + indices[k++];
        v1 = points + indices[k++];
        v2 = points + indices[k++];

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

int main(int argc, char **argv) {
    Wing wing = {
        .airfoil = {M, P, T},
        .semi_span = SEMI_SPAN,
        .root_chord = ROOT_CHORD,
        .sweep_angle_leading = LEADING_SWEEP,
        .sweep_angle_trailing = TRAILING_SWEEP,
        .has_cosine_chord_spacing = USE_COSINE_SPACING,
        .has_closed_trailing_edge = CLOSE_TRAILING_EDGE,
        .num_points_spanwise = NUM_PANELS_SPANWISE + 1,
        .num_points_chordwise = NUM_PANELS_CHORDWISE + 1,
    };

    Vector3D *points = create_points(&wing);

    if (points == NULL) {
        fprintf(stderr, "wingstl: error: unable to allocate memory for surface vertices\n");

        return 1;
    }

    size_t *indices = create_indices(&wing);
    create_indices(&wing);

    if (indices == NULL) {
        fprintf(stderr, "wingstl: error: unable to allocate memory for triangle indices\n");
        free(points);

        return 1;
    }

    write_to_stl(points, indices, compute_num_triangles(&wing), "wing.stl");
    
    free(indices);
    free(points);

    return 0;
}

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.