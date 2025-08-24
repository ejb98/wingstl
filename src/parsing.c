/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "types.h"
#include "messages.h"
#include "constants.h"

void request_val(const char *desc, const char *flag) {
    printf("wingstl: value required for %s (flag '%s')\n", desc, flag);
}

void request_nonzero_pos_num(const char *desc, const char *flag) {
    printf("wingstl: nonzero positive number required for %s (flag '%s')\n", desc, flag);
}

void request_n_digit_int(const char *desc, const char *flag, int n) {
    char noun[] = "digits";
    int len = strlen(noun);

    if (n == 1) {
        noun[len - 1] = '\0';
    }

    printf("wingstl: value for %s (flag '%s') must be an integer with exactly %d %s\n", desc, flag, n, noun);
}

void request_bounded_int(const char *desc, const char *flag, int val, const char *qualifier) {
    printf("wingstl: value for %s (flag '%s') must be %d %s\n", desc, flag, val, qualifier);
}

float handle_nonzero_pos_num(int iarg, int num_args, char **args, const char *desc, const char *flag) {
    float value = -1.0f;

    if (iarg + 1 < num_args) {
        value = atof(args[iarg + 1]);

        if (value <= 0.0f) {
            request_nonzero_pos_num(desc, flag);
            return -1.0f;
        }

    } else {
        request_val(desc, flag);
        return -1.0f;
    }

    return value;
}

Units handle_units(int iarg, int num_args, char **args) {
    Units units = INVALID;

    if (iarg + 1 < num_args) {
        char *arg = args[iarg + 1];

        if (strcmp(arg, "m") == 0) {
            return METERS;
        } else if (strcmp(arg, "cm") == 0) {
            return CENTIMETERS;
        } else if (strcmp(arg, "mm") == 0) {
            return MILLIMETERS;
        } else if (strcmp(arg, "ft") == 0) {
            return FEET;
        } else if (strcmp(arg, "in") == 0) {
            return INCHES;
        } else {
            printf("wingstl: valid options for units (flag '%s') are: ", FLAG_UNITS);
            printf("'m', 'cm', 'mm', 'ft' or 'in'\n");
            return INVALID;
        }

    } else {
        request_val("units", FLAG_UNITS);
        return INVALID;
    }

    return units;
}

NACA4Digit handle_airfoil(int iarg, int num_args, char **args) {
    NACA4Digit airfoil = {.m = -1};

    if (iarg + 1 < num_args) {
        char *arg = args[iarg + 1];
        int num_digits = strlen(arg);

        if (num_digits != 4) {
            request_n_digit_int("naca airfoil", FLAG_AIRFOIL, 4);
            airfoil.m = -1;
            return airfoil;
        }

        char digit;
        for (int j = 0; j < num_digits; j++) {
            digit = arg[j];

            if (!isdigit(digit)) {
                request_n_digit_int("naca airfoil", FLAG_AIRFOIL, 4);
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
                    request_n_digit_int("naca airfoil", FLAG_AIRFOIL, 4);
                    airfoil.m = -1;
                    return airfoil;
            }
        }

    } else {
        request_val("naca 4-digit airfoil", FLAG_AIRFOIL);
        airfoil.m = -1;
        return airfoil;
    }

    return airfoil;
}

int handle_chord_pts(int iarg, int num_args, char **args) {
    int num_pts = -1;
    char desc[] = "number of chordwise points";

    if (iarg + 1 < num_args) {
        char *arg = args[iarg + 1];
        num_pts = atoi(arg);

        if (num_pts < MIN_CHORD_PTS) {
            request_bounded_int(desc, FLAG_CHORD_PTS, MIN_CHORD_PTS, "at least");
            return -1;
        }

        if (num_pts > MAX_CHORD_PTS) {
            request_bounded_int(desc, FLAG_CHORD_PTS, MAX_CHORD_PTS, "at most");
            return -1;
        }

    } else {
        request_val(desc, FLAG_CHORD_PTS);
        return -1;
    }

    return num_pts;
}

float handle_sweep(int iarg, int num_args, char **args, const char *arg_flag) {
    char desc[32];
    float sweep = -1.0f;

    bool is_le = strcmp(arg_flag, FLAG_SWEEP_LE) == 0;
    snprintf(desc, sizeof(desc), "%s edge sweep angle", is_le ? "leading" : "trailing");

    if (iarg + 1 < num_args) {
        char *arg = args[iarg + 1];
        sweep = atof(arg);

        if (sweep <= 0.0f) {
            request_nonzero_pos_num(desc, arg_flag);
            return -1.0f;
        }

        if (sweep < MIN_SWEEP) {
            request_bounded_int(desc, arg_flag, MIN_SWEEP, "at least");
            return -1.0f;
        }

        if (sweep > MAX_SWEEP) {
            request_bounded_int(desc, arg_flag, MAX_SWEEP, "at most");
            return -1.0f;
        }

    } else {
        request_val(desc, arg_flag);
        return -1.0f;
    }

    return sweep;
}

int handle_inputs(int num_args, char **args, Wing *wing, Settings *settings) {
    if (num_args == 1) {
        printf("wingstl: missing required arguments; use flag ('%s') for help\n", FLAG_HELP);
        return 1;
    }

    char *arg = NULL;

    for (int i = 1; i < num_args; i++) {
        arg = args[i];

        if (arg[0] != '-') {
            printf("wingstl: argument flags must begin with a hyphen '-'\n");
            return 1;
        }

        if (strcmp(arg, FLAG_VERBOSE) == 0) {
            settings->verbose = true;

        } else if (strcmp(arg, FLAG_HELP) == 0) {
            show_help();
            return 1;

        } else if (strcmp(arg, FLAG_SEMI_SPAN) == 0) {
            wing->semi_span = handle_nonzero_pos_num(i, num_args, args, "semi span", FLAG_SEMI_SPAN);
            if (wing->semi_span < 0.0f) { return 1; } else { i++; }

        } else if (strcmp(arg, FLAG_ROOT_CHORD) == 0) {
            wing->root_chord = handle_nonzero_pos_num(i, num_args, args, "root chord", FLAG_ROOT_CHORD);
            if (wing->root_chord < 0.0f) { return 1; } else { i++; }

        } else if (strcmp(arg, FLAG_AIRFOIL) == 0) {
            wing->airfoil = handle_airfoil(i, num_args, args);
            if (wing->airfoil.m < 0) { return 1; } else { i++; }
            
        } else if (strcmp(arg, FLAG_CHORD_PTS) == 0) {
            wing->num_pts_chord = handle_chord_pts(i, num_args, args);
            if (wing->num_pts_chord < 0) { return 1; } else { i++; }

        } else if (strcmp(arg, FLAG_SWEEP_LE) == 0) {
            wing->sweep_angles[0] = handle_sweep(i, num_args, args, FLAG_SWEEP_LE);
            if (wing->sweep_angles[0] < 0.0f) { return 1; } else { i++; }

        } else if (strcmp(arg, FLAG_SWEEP_TE) == 0) {
            wing->sweep_angles[1] = handle_sweep(i, num_args, args, FLAG_SWEEP_TE);
            if (wing->sweep_angles[1] < 0.0f) { return 1; } else { i++; }

        } else if (strcmp(arg, FLAG_UNITS) == 0) {
            wing->units = handle_units(i, num_args, args);
            if (wing->units == INVALID) { return 1; } else { i++; }

        } else {
            printf("wingstl: unrecognized argument flag '%s'\n", arg);
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
