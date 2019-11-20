# STEP Slicer Engine
An experimental 3D model slicer engine, built on top of a CAD kernel

## Why?
* No loss of precision from CAD → STL conversion
* Retain model metadata (units, precision, color)
* Non-planar slicing
* Support for more advanced Gcode words and macros
* Additive + subtractive manufacturing
* Better multi-object support
* Better multi-material support
* Edit model in-slicer
* Exportable support structures
* more accurate FEM simulation

## Dependencies
* [OpenCasCade](https://www.opencascade.com/) > 7.3.0
* [OpenMP](https://www.openmp.org/)

## Supported CNC software:
* [LinuxCNC](http://linuxcnc.org/)
* [Machinekit](https://www.machinekit.io/)
* [Redeem](http://wiki.thing-printer.com/index.php?title=Redeem)

## License
AGPL

## Build Instructions
```
git clone --recursive https://github.com/karl-nilsson/sse.git && cd sse
mkdir build && cd build
cmake ..
make
sudo make install
```
