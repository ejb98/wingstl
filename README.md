# wingstl
This project is a CLI tool for generating print-ready STL files for swept wings.

## Build Instructions
This project uses a single C source file, and is not dependent on any third-party libraries. To compile use:

<pre> ```clang wingstl.c -o wingstl``` </pre>

To execute the script, add the location of the executable to your system's path and run:

<pre> ```wingstl``` </pre>

The output is a single STL file saved to the current directory. This file may be directly imported to slicer software for 3D printing.

## References
This project uses methods for geometry generation provided in sources [1] and [2]:

1. Ladson, Charles L., et al. Computer program to obtain ordinates for NACA airfoils. No. L-17509. 1996.
2. Katz, Joseph, and Allen Plotkin. Low-speed aerodynamics. Vol. 13. Cambridge university press, 2001.

## License
Licensed under the GNU General Public License v3.0 or later (GPLv3+). See end of source file for details.

## Future Work
This project is ongoing. Future work will involve adding various argument flags to allow more granular control over user input
without the need to recompile after each change.