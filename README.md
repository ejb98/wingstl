# wingstl
## Overview
`wingstl` is a lightweight command-line tool for generating 3D-printable STL files of swept wings based on NACA 4-digit airfoils.
It allows you to quickly define wing geometry and export it directly for use in CAD, slicing software, or CFD preprocessing.

![Alt text](images/wing.png)

*Screenshot of example output STL model*

## Documentation
### Build Instructions
This project is structured with `include` and `src` folders, and is not dependent on any third-party libraries. To build with `clang`:
```bash
clang -O2 -std=c99 src/*.c -Iinclude -o wingstl
```

### Usage
```bash
wingstl [OPTIONS]
```
Generate an STL file for a swept wing given user-defined dimensions and NACA airfoil code.

### Options

| Option | Type | Default      | Description                                                                            |
| ------ | ---- | ------------ | -------------------------------------------------------------------------------------- |
| `-h`   | flag | *(n/a)*      | Show the help message and exit.                                                        |
| `-v`   | flag | *(n/a)*      | Enable verbose output.                                                                 |
| `-p`   | INT  | `100`        | Number of coordinate points along the chord (higher = smoother STL, larger file size). |
| `-a`   | STR  | *(required)* | NACA 4-digit airfoil code (e.g., `2412`, `0012`).                                      |
| `-u`   | STR  | `"m"`        | Units for all dimensions (`m`, `cm`, `mm`, `ft`, or `in`).                             |
| `-o`   | STR  | `"wing.stl"` | Output STL file name.                                                                  |
| `-b`   | REAL | *(required)* | Wing **semi-span length** (distance from root to tip).                                 |
| `-c`   | REAL | *(required)* | Wing **root chord length**.                                                            |
| `-l`   | REAL | `90`         | **Leading edge sweep angle** in degrees (90° = no sweep).                              |
| `-t`   | REAL | `90`         | **Trailing edge sweep angle** in degrees (90° = no sweep).                             |

### Examples
Generate a 6 m semi-span wing with a 1 m root chord using a NACA 2412 airfoil:
```bash
wingstl -a 2412 -b 6 -c 1 -v -o planform.stl
```
Generate a smaller wing in feet with a swept leading and trailing edge:
```bash
wingstl -a 1224 -b 3 -c 0.75 -u ft -l 85 -t 85

```
### Output
The tool produces a standard `.stl` file that can be:
* Imported into CAD software for further modification
* Loaded into a slicer for direct 3D printing
* Used in CFD/FEA workflows for aerodynamic or structural analysis

## References
This project uses methods for geometry generation as described in the following references:
1. Ladson, Charles L., et al. Computer program to obtain ordinates for NACA airfoils. No. L-17509. 1996.
2. Katz, Joseph, and Allen Plotkin. Low-speed aerodynamics. Vol. 13. Cambridge university press, 2001.

## License
Licensed under the GNU General Public License v3.0 or later (GPLv3+). See LICENSE file for details.