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
#include "utils.h"
#include "constants.h"

void request_value(const char *desc, const char *flag) {
    fprintf(stderr, "wingstl: error: value required for %s (flag '%s')\n", desc, flag);
}

void request_nonzero_positive(const char *desc, const char *flag) {
    fprintf(stderr, "wingstl: error: nonzero positive number required for %s (flag '%s')\n", desc, flag);
}

void request_n_digits(const char *desc, const char *flag, int n) {
    char noun[] = "digits";
    int len = strlen(noun);

    if (n == 1) {
        noun[len - 1] = '\0';
    }

    fprintf(stderr, "wingstl: error: value for %s (flag '%s') must be a number with exactly %d %s\n", desc, flag, n, noun);
}

void request_bounded_integer(const char *desc, const char *flag, int val, const char *qualifier) {
    fprintf(stderr, "wingstl: error: value for %s (flag '%s') must be %d %s\n", desc, flag, val, qualifier);
}

float handle_nonzero_positive(int iarg, int num_args, char **args, const char *desc, const char *flag) {
    float value = -1.0f;

    if (iarg + 1 < num_args) {
        value = atof(args[iarg + 1]);

        if (value <= 0.0f) {
            request_nonzero_positive(desc, flag);
            return -1.0f;
        }

    } else {
        request_value(desc, flag);
        return -1.0f;
    }

    return value;
}

Units handle_units(int iarg, int num_args, char **args) {
    Units units = INVALID;

    if (iarg + 1 < num_args) {
        char *arg = args[iarg + 1];

        units = to_units(arg);

        if (units == INVALID) {
            fprintf(stderr, "wingstl: error: valid options for units (flag '%s') are: ", FLAG_UNITS);
            fprintf(stderr, "'m', 'cm', 'mm', 'ft' or 'in'\n");

            return INVALID;
        }

    } else {
        request_value("units", FLAG_UNITS);

        return INVALID;
    }

    return units;
}

char *handle_output(int iarg, int num_args, char **args) {
    char *output = NULL;

    if (iarg + 1 < num_args) {
        char *arg = args[iarg + 1];
        int num_chars = strlen(arg);
        char *last_four = arg + num_chars - 4;
        bool has_ext = (strcmp(last_four, ".stl") == 0 || strcmp(last_four, ".STL") == 0);

        if (!has_ext) {
            num_chars += 4;
        }

        output = (char *) malloc((num_chars + 1) * sizeof(char));

        if (output == NULL) {
            fprintf(stderr, "wingstl: error: unable to allocate memory for file output\n");
            return NULL;
        }

        if (has_ext) {
            strcpy(output, arg);
        } else {
            sprintf(output, "%s.stl", arg);
        }

    } else {
        request_value("output file", FLAG_OUTPUT);
        return NULL;
    }

    return output;
}

NACA4Digit handle_airfoil(int iarg, int num_args, char **args) {
    NACA4Digit airfoil = {.m = -1};

    if (iarg + 1 < num_args) {
        char *arg = args[iarg + 1];
        int num_digits = strlen(arg);

        if (num_digits != 4) {
            request_n_digits("naca airfoil", FLAG_AIRFOIL, 4);
            airfoil.m = -1;
            return airfoil;
        }

        if (arg[2] == '0' && arg[3] == '0') {
            fprintf(stderr, "wingstl: error: argument for flag '%s' will result in zero thickness;\n", FLAG_AIRFOIL);
            fprintf(stderr, "                try increasing either of the last two digits of '%s'\n", arg);
            fprintf(stderr, "                                                                   ^^");
            airfoil.m = -1;
            return airfoil;
        }

        char digit;
        for (int j = 0; j < num_digits; j++) {
            digit = arg[j];

            if (!isdigit(digit)) {
                request_n_digits("naca airfoil", FLAG_AIRFOIL, 4);
                airfoil.m = -1;
                return airfoil;
            }

            switch (j) {
                case 0:
                    airfoil.m = to_integer(digit);
                    break;
                case 1:
                    airfoil.p = to_integer(digit);
                    break;
                case 2:
                    airfoil.t = to_integer(digit);
                    break;
                case 3:
                    airfoil.t = 10 * airfoil.t + to_integer(digit);
                    break;
                default:
                    request_n_digits("naca airfoil", FLAG_AIRFOIL, 4);
                    airfoil.m = -1;
                    return airfoil;
            }
        }

    } else {
        request_value("naca 4-digit airfoil", FLAG_AIRFOIL);
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
            request_bounded_integer(desc, FLAG_CHORD_PTS, MIN_CHORD_PTS, "at least");
            return -1;
        }

        if (num_pts > MAX_CHORD_PTS) {
            request_bounded_integer(desc, FLAG_CHORD_PTS, MAX_CHORD_PTS, "at most");
            return -1;
        }

    } else {
        request_value(desc, FLAG_CHORD_PTS);
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
            request_nonzero_positive(desc, arg_flag);
            return -1.0f;
        }

        if (sweep < MIN_SWEEP) {
            request_bounded_integer(desc, arg_flag, MIN_SWEEP, "at least");
            return -1.0f;
        }

        if (sweep > MAX_SWEEP) {
            request_bounded_integer(desc, arg_flag, MAX_SWEEP, "at most");
            return -1.0f;
        }

    } else {
        request_value(desc, arg_flag);
        return -1.0f;
    }

    return sweep;
}

int handle_inputs(int num_args, char **args, Wing *wing, Settings *settings) {
    if (num_args == 1) {
        fprintf(stderr, "wingstl: error: missing required arguments; use flag ('%s') for help\n", FLAG_HELP);
        return 1;
    }

    char *arg = NULL;

    for (int i = 1; i < num_args; i++) {
        arg = args[i];

        if (arg[0] != '-') {
            fprintf(stderr, "wingstl: error: argument flags must begin with a hyphen '-'\n");
            return 1;
        }

        if (strcmp(arg, FLAG_VERBOSE) == 0) {
            settings->verbose = true;

        } else if (strcmp(arg, FLAG_HELP) == 0) {
            settings->help = true;
            return 1;

        } else if (strcmp(arg, FLAG_OUTPUT) == 0) {
            settings->output = handle_output(i, num_args, args);
            if (settings->output == NULL) { return 1; } else { i++; }

        } else if (strcmp(arg, FLAG_SEMI_SPAN) == 0) {
            wing->semi_span = handle_nonzero_positive(i, num_args, args, "semi span", FLAG_SEMI_SPAN);
            if (wing->semi_span < 0.0f) { return 1; } else { i++; }

        } else if (strcmp(arg, FLAG_ROOT_CHORD) == 0) {
            wing->root_chord = handle_nonzero_positive(i, num_args, args, "root chord", FLAG_ROOT_CHORD);
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
