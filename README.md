# StepSlicerEngine
This project aims to be a next-gen slicing engine

## WARNING! THIS IS EXPERIMENTAL SOFTWARE! USE AT YOUR OWN RISK!

## Features
* Accepts a wide variety of 3D filetypes:
** STL
** OBJ
** BRep
** STEP
** IGES
* Adaptive slicing mode
* Manual feature mode
* Writes complex G-Code:
** G5 splines and curves
** o100 loops


## Building
# Dependencies
* cmake
* [OCCT](https://www.opencascade.com/) >= 7.3.0
* Boost

# Build
cmake .
make

## Usage
sse [-h] [-v] [slicing_options] [printer_config] file1 file2 ...
-h help
-v version

## License
StepSlicerEngine is released under...TODO
