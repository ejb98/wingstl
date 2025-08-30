# wingstl
`wingstl` is a lightweight command-line tool for generating 3D-printable STL files of swept wings using a specified airfoil.
It allows you to quickly define wing geometry and export it directly for use in CAD, slicing software, or CFD preprocessing.

## Example Output
![Alt text](images/wing.png)
*Screenshot of example output STL model*

## Releases
Precompiled binaries are available under the [Releases](https://github.com/ejb98/wingstl/releases/latest) section of this repository.

## Documentation
### Build Instructions
This project is structured with `include` and `src` folders, and is not dependent on any third-party libraries. To build with `clang`:
```bash
clang -O2 -std=c99 src/*.c -Iinclude -o wingstl
```
Note: also use `-lm` flag if building on Linux.

### Usage
```bash
wingstl [OPTIONS]
```
Generate an STL file for a swept wing given user-defined dimensions and NACA airfoil code (or .dat file name).

### Options

| Option | Type | Default      | Description                                                                            |
| ------ | ---- | ------------ | -------------------------------------------------------------------------------------- |
| `-h`   | flag | *(n/a)*      | Show the help message and exit.                                                        |
| `-v`   | flag | *(n/a)*      | Enable verbose output.                                                                 |
| `-p`   | INT  | `100`        | Number of coordinate points along the chord (higher = smoother STL, larger file size). |
| `-a`   | STR  | *(required)* | NACA 4-digit airfoil code (e.g., `2412`, `0012`) or .dat file name.                    |
| `-u`   | STR  | `"m"`        | Units for all dimensions (`m`, `cm`, `mm`, `ft`, or `in`).                             |
| `-o`   | STR  | `"wing.stl"` | Output STL file name.                                                                  |
| `-b`   | REAL | *(required)* | Wing **semi-span length** (distance from root to tip).                                 |
| `-c`   | REAL | *(required)* | Wing **root chord length**.                                                            |
| `-l`   | REAL | `90`         | **Leading edge sweep angle** in degrees (90° = no sweep).                              |
| `-t`   | REAL | `90`         | **Trailing edge sweep angle** in degrees (90° = no sweep).                             |

### Examples
Generate a 6 meter semi-span wing with a 1 m root chord using a NACA 2412 airfoil:
```bash
wingstl -a 2412 -b 6 -c 1 -v -o planform.stl
```
Generate a 3 foot semi-span wing with swept leading and trailing edges and using a Selig 1223 airfoil:
```bash
wingstl -a selig_1223.dat -b 3 -c 0.75 -u ft -l 85 -t 85

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