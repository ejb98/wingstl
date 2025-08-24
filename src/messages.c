/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#include <stdio.h>
#include <string.h>

#include "types.h"
#include "engine.h"
#include "constants.h"

void show_help(void) {
    printf("Usage: wingstl [OPTIONS]\n\n");
    printf("Generate an STL of a wing given dimensions and NACA airfoil.\n\n");
    printf("Options:\n");
    printf("  -h\t\tShow this help message and exit\n");
    printf("  -v\t\tEnable verbose output\n");
    printf("  -p  INT\tNumber of points along the chord (default: %d)\n", DEFAULT_NUM_CHORD_PTS);
    printf("  -a  STR\tNACA 4-digit airfoil code (required)\n");
    printf("  -b  REAL\tSemi span length (required)\n");
    printf("  -c  REAL\tRoot chord length (required)\n");
    printf("  -u  STR\tUnits (default: m)\n");
    printf("  -le REAL\tLeading edge sweep angle in degrees (default: %.0f)\n", DEFAULT_SWEEP_LE);
    printf("  -te REAL\tTrailing edge sweep angle in degrees (default: %.0f)\n\n", DEFAULT_SWEEP_TE);
    printf("Examples:\n");
    printf("  wingstl -a 2412 -b 6 -c 1 -v\n");
    printf("  wingstl -a 1224 -b 3 -c 0.75 -u in -le 85 -te 85\n\n");
    printf("Report bugs to: github.com/ejb98/wingstl\n");
}

void show_wing_props(const Wing *wing) {
    char units[16];

    switch (wing->units) {
        case METERS:
            strcpy(units, "meters");
            break;
        case CENTIMETERS:
            strcpy(units, "centimeters");
            break;
        case MILLIMETERS:
            strcpy(units, "millimeters");
            break;
        case FEET:
            strcpy(units, "feet");
            break;
        case INCHES:
            strcpy(units, "inches");
            break;
        case INVALID:
            strcpy(units, "units");
            break;
        default:
            strcpy(units, "units");
    }

    printf("Wing properties:\n");
    printf("  Semi span length:\t\t%.2f %s\n", wing->semi_span, units);
    printf("  Root chord length:\t\t%.2f %s\n", wing->root_chord, units);
    printf("  Airfoil profile:\t\tNACA %d%d%d\n", wing->airfoil.m, wing->airfoil.p, wing->airfoil.t);
    printf("  Full wing aspect ratio:\t%.2f\n", get_aspect_ratio(wing));
    printf("  Full wing surface area:\t%.2f square %s\n", get_surf_area(wing), units);
    printf("  Leading edge sweep angle:\t%.2f degrees\n", wing->sweep_angles[0]);
    printf("  Trailing edge sweep angle:\t%.2f degrees\n", wing->sweep_angles[1]);
    printf("  Trailing edge configuration:\t%s\n", (wing->has_closed_te ? "closed" : "open"));
    printf("  Spanwise points:\t\t%d\n", wing->num_pts_span);
    printf("  Chordwise points:\t\t%d\n", wing->num_pts_chord);
    printf("  Chordwise distribution:\t%s\n", (wing->has_cosine_spacing ? "cosine" : "linear"));
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