# STEP Slicer Engine

## Introduction
STEP Slicer Engine is an experimental 3D model slicer engine, built on top of a fully-featured CAD kernel

## Why?
* No more loss of precision from CAD → STL conversion
* Retain model metadata (units, precision, color)
* Non-planar slicing
* Support for more advanced Gcode words and macros
* Better multi-object support
* Better multi-material support
* Edit model in-slicer
* Exportable support structures
* FEM simulation

## Dependencies
* [OpenCasCade](https://www.opencascade.com/)

## Supported CNC software:
* [LinuxCNC](http://linuxcnc.org/)
* [Machinekit](https://www.machinekit.io/)
* [Redeem](http://wiki.thing-printer.com/index.php?title=Redeem)

## Build Instructions
```
git clone https://github.com/karl-nilsson/sse.git && cd sse
mkdir build && cd build
cmake ..
make
sudo make install
```
