/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "types.h"
#include "constants.h"

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

        if (sweep <= MIN_SWEEP || sweep >= MAX_SWEEP) {
            fprintf(stderr, "wingstl: error: value for option %s must be a number betwen %.0f - %.0f (exclusive)\n",
                    arg_flag , MIN_SWEEP, MAX_SWEEP);

            return -1.0f;
        }

    } else {
        fprintf(stderr, "wingstl: error: option %s (leading edge sweep angle) requires a value (e.g., 80.0)\n", arg_flag);

        return -1.0f;
    }

    return sweep;
}

int handle_inputs(int num_args, char **args, wing3d *wing) {
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
            wing->sweep_angles[1] = handle_sweep(i, num_args, args, FLAG_SWEEP_TE);
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
