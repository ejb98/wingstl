/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#include <stdio.h>
#include <string.h>

#include "types.h"
#include "engine.h"
#include "messages.h"
#include "constants.h"

void show_help(void) {
    printf("Usage: wingstl [OPTIONS]\n\n");
    printf("Generate an STL file for a swept wing given dimensions and airfoil.\n\n");

    printf("Options:\n");
    printf("  %s\t\tShow this help message and exit\n", FLAG_HELP);
    printf("  %s\t\tEnable verbose output\n", FLAG_VERBOSE);
    printf("  %s  INT\tNumber of desired spanwise slices for STL model (default: %d)\n", FLAG_NUM_SLICES, DEFAULT_NUM_SLICES);
    printf("  %s  INT\tNumber of points along the chord (default: %d)\n", FLAG_CHORD_PTS, DEFAULT_NUM_CHORD_PTS);
    printf("  %s  STR\tNACA 4-digit airfoil code or airfoil .dat file name (required)\n", FLAG_AIRFOIL);
    printf("  %s  STR\tUnits (default: '%s')\n", FLAG_UNITS, DEFAULT_UNITS);
    printf("  %s  STR\tOutput file name (default: '%s')\n", FLAG_OUTPUT, DEFAULT_OUTPUT);
    printf("  %s  REAL\tSemi span length (required)\n", FLAG_SEMI_SPAN);
    printf("  %s  REAL\tRoot chord length (required)\n", FLAG_ROOT_CHORD);
    printf("  %s  REAL\tLeading edge sweep angle in degrees (default: %.0f)\n", FLAG_SWEEP_LE, DEFAULT_SWEEP_LE);
    printf("  %s  REAL\tTrailing edge sweep angle in degrees (default: %.0f)\n\n", FLAG_SWEEP_TE, DEFAULT_SWEEP_TE);

    printf("Examples:\n");
    printf("  wingstl %s 2412 %s 6 %s 1 %s %s 4 %s planform.stl\n", 
              FLAG_AIRFOIL, FLAG_SEMI_SPAN, FLAG_ROOT_CHORD, FLAG_VERBOSE, FLAG_NUM_SLICES, FLAG_OUTPUT);
    printf("  wingstl %s selig_1223.dat %s 3 %s 0.75 %s ft %s 85 %s 85\n\n", 
              FLAG_AIRFOIL, FLAG_SEMI_SPAN, FLAG_ROOT_CHORD, FLAG_UNITS, FLAG_SWEEP_LE, FLAG_SWEEP_TE);

    printf("Report bugs to: github.com/ejb98/wingstl\n");
}

void show_settings(const Settings *settings) {
    char units[8];

    switch (settings->units) {
        case METERS:
            strcpy(units, "m");
            break;
        case CENTIMETERS:
            strcpy(units, "cm");
            break;
        case MILLIMETERS:
            strcpy(units, "mm");
            break;
        case FEET:
            strcpy(units, "ft");
            break;
        case INCHES:
            strcpy(units, "in");
            break;
        case UNKNOWN_UNITS:
            strcpy(units, "units");
            break;
        default:
            strcpy(units, "units");
    }

    printf("Wing properties:\n");
    printf("  Semi span length:\t\t%.2f %s\n", settings->semi_span, units);
    printf("  Root chord length:\t\t%.2f %s\n", settings->root_chord, units);
    printf("  Airfoil profile:\t\t");

    if (!settings->airfoil.num_pts) {
        printf("NACA %s\n", settings->airfoil.header);
    } else {
        printf("%s\n", settings->airfoil.header);
    }

    printf("  Full wing aspect ratio:\t%.2f\n", get_aspect_ratio(settings));
    printf("  Full wing surface area:\t%.2f sq %s\n", get_surface_area(settings), units);
    printf("  Leading edge sweep angle:\t%.2f deg\n", settings->sweep_angles[0]);
    printf("  Trailing edge sweep angle:\t%.2f deg\n", settings->sweep_angles[1]);
    printf("  Trailing edge configuration:\t%s\n", (settings->airfoil.has_closed_te ? "closed" : "open"));
    printf("  Chordwise points:\t\t%d\n", settings->num_pts_chord);
    printf("  Number of slices:\t\t%d\n", settings->num_slices);
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