/*
 * Copyright (C) 2025 Ethan Billingsley
 * License: GPLv3 (see end of file for full notice, or LICENSE file in repo)
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

#define A0 0.2969f
#define A1 -0.126f
#define A2 -0.3516f
#define A3 0.2843f
#define A4_OPEN -0.1015f
#define A4_CLOSED -0.1036f

#define PI 3.14159f
#define PI_OVER_180 0.01745f

#define MIN_SWEEP 1
#define MAX_SWEEP 179
#define MIN_CHORD_PTS 20
#define MAX_CHORD_PTS 200
#define MIN_ASPECT_RATIO 1.0f
#define MAX_ASPECT_RATIO 100.0f

#define FLAG_HELP "-h"
#define FLAG_UNITS "-u"
#define FLAG_OUTPUT "-o"
#define FLAG_AIRFOIL "-a"
#define FLAG_VERBOSE "-v"
#define FLAG_SWEEP_LE "-l"
#define FLAG_SWEEP_TE "-t"
#define FLAG_CHORD_PTS "-p"
#define FLAG_SEMI_SPAN "-b"
#define FLAG_ROOT_CHORD "-c"

#define DEFAULT_UNITS "m"
#define DEFAULT_OUTPUT "wing.stl"
#define DEFAULT_AIRFOIL -1
#define DEFAULT_SWEEP_LE 90.0f
#define DEFAULT_SWEEP_TE 90.0f
#define DEFAULT_SEMI_SPAN -1.0f
#define DEFAULT_ROOT_CHORD -1.0f
#define DEFAULT_NUM_SPAN_PTS 2
#define DEFAULT_NUM_CHORD_PTS 100
#define DEFAULT_HAS_CLOSED_TE 1
#define DEFAULT_HAS_COSINE_SPACING 1

#define FEET_PER_METER 3.28084f
#define INCHES_PER_METER 39.3701f

#endif

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