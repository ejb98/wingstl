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
#include "fileio.h"
#include "parsing.h"
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
    Units units = UNKNOWN_UNITS;

    if (iarg + 1 < num_args) {
        char *arg = args[iarg + 1];

        units = to_units(arg);

        if (units == UNKNOWN_UNITS) {
            fprintf(stderr, "wingstl: error: valid options for units (flag '%s') are: ", FLAG_UNITS);
            fprintf(stderr, "'m', 'cm', 'mm', 'ft' or 'in'\n");

            return UNKNOWN_UNITS;
        }

    } else {
        request_value("units", FLAG_UNITS);

        return UNKNOWN_UNITS;
    }

    return units;
}

char *handle_output(int iarg, int num_args, char **args) {
    char *output = NULL;

    if (iarg + 1 < num_args) {
        char *arg = args[iarg + 1];
        int length = strlen(arg);
        bool has_ext = (strstr(arg, ".stl") != NULL) || (strstr(arg, ".STL") != NULL);

        if (!has_ext) {
            length += 4;
        }

        output = (char *) malloc((length + 1) * sizeof(char));

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

void handle_airfoil(int iarg, int num_args, char **args, Airfoil *airfoil) {
    airfoil->num_pts = -1;

    if (iarg + 1 < num_args) {
        char *arg = args[iarg + 1];
        bool has_ext = (strstr(arg, ".dat") != NULL) || (strstr(arg, ".DAT") != NULL);

        if (has_ext) {
            if (!read_dat(arg, airfoil)) {
                return;
            }

            airfoil->num_pts = -1;
            return;
        }

        int length = strlen(arg);
        bool is_four_digits = (length == 4);

        for (int i = 0; i < length; i++) {
            is_four_digits = is_four_digits && isdigit((unsigned char) arg[i]);
        }

        if (is_four_digits) {
            bool has_zero_thickness = (arg[2] == '0' && arg[3] == '0');

            if (!has_zero_thickness) {
                airfoil->num_pts = 0;
                airfoil->has_closed_te = true;
                strcpy(airfoil->header, arg);
                return;
            }

            fprintf(stderr, "wingstl: error: argument for flag '%s' will result in zero thickness; ", FLAG_AIRFOIL);
            fprintf(stderr, "try increasing either of the last two digits of '%s'\n", arg);
            airfoil->num_pts = -1;
            return;
        }

        char *output = (char *) malloc((length + 5) * sizeof(char));

        if (output == NULL) {
            fprintf(stderr, "wingstl: error: unable to allocate memory for file input\n");
            airfoil->num_pts = -1;
            return;
        }

        sprintf(output, "%s.dat", arg);
        FILE *f = fopen(output, "r");

        if (f == NULL) {
            sprintf(output, "%s.DAT", arg);
            f = fopen(output, "r");
        }
        
        if (f == NULL) {
            fprintf(stderr, "wingstl: error: argument for flag '%s' must be either a ", FLAG_AIRFOIL);
            fprintf(stderr, "4-digit naca code or a valid .dat file name\n");
            airfoil->num_pts = -1;
            free(output);
            return;
        }

        fclose(f);

        if (!read_dat(output, airfoil)) {
            free(output);
            return;
        }

        free(output);
        airfoil->num_pts = -1;
        return;
    } else {
        request_value("airfoil .dat file or 4-digit naca code", FLAG_AIRFOIL);
        airfoil->num_pts = -1;
        return;
    }
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

int handle_num_slices(int iarg, int num_args, char **args) {
    int num_slices = -1;
    char desc[] = "number of model slices";

    if (iarg + 1 < num_args) {
        char *arg = args[iarg + 1];
        num_slices = atoi(arg);

        if (num_slices < MIN_NUM_SLICES) {
            request_bounded_integer(desc, FLAG_NUM_SLICES, MIN_NUM_SLICES, "at least");
            return -1;
        }

        if (num_slices > MAX_NUM_SLICES) {
            request_bounded_integer(desc, FLAG_NUM_SLICES, MAX_NUM_SLICES, "at most");
            return -1;
        }

    } else {
        request_value(desc, FLAG_NUM_SLICES);
        return -1;
    }

    return num_slices;
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

int handle_inputs(int num_args, char **args, Settings *settings) {
    if (num_args < 2) {
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
            settings->semi_span = handle_nonzero_positive(i, num_args, args, "semi span", FLAG_SEMI_SPAN);
            if (settings->semi_span < 0.0f) { return 1; } else { i++; }

        } else if (strcmp(arg, FLAG_ROOT_CHORD) == 0) {
            settings->root_chord = handle_nonzero_positive(i, num_args, args, "root chord", FLAG_ROOT_CHORD);
            if (settings->root_chord < 0.0f) { return 1; } else { i++; }

        } else if (strcmp(arg, FLAG_AIRFOIL) == 0) {
            handle_airfoil(i, num_args, args, &settings->airfoil);
            if (settings->airfoil.num_pts < 0) { return 1; } else { i++; }
            
        } else if (strcmp(arg, FLAG_CHORD_PTS) == 0) {
            settings->num_pts_chord = handle_chord_pts(i, num_args, args);
            if (settings->num_pts_chord < 0) { return 1; } else { i++; }

        } else if (strcmp(arg, FLAG_NUM_SLICES) == 0) {
            settings->num_slices = handle_num_slices(i, num_args, args);
            if (settings->num_slices < 0) { return 1; } else { i++; }

        } else if (strcmp(arg, FLAG_SWEEP_LE) == 0) {
            settings->sweep_angles[0] = handle_sweep(i, num_args, args, FLAG_SWEEP_LE);
            if (settings->sweep_angles[0] < 0.0f) { return 1; } else { i++; }

        } else if (strcmp(arg, FLAG_SWEEP_TE) == 0) {
            settings->sweep_angles[1] = handle_sweep(i, num_args, args, FLAG_SWEEP_TE);
            if (settings->sweep_angles[1] < 0.0f) { return 1; } else { i++; }

        } else if (strcmp(arg, FLAG_UNITS) == 0) {
            settings->units = handle_units(i, num_args, args);
            if (settings->units == UNKNOWN_UNITS) { return 1; } else { i++; }

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
