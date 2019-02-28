# STEP Slicer Engine

## Introduction
STEP Slicer Engine is an experimental 3D model slicer engine, built on top of a fully-featured CAD kernel

## Why?
* No more loss of precision from CAD -> STL conversion
* Retain unit info
* Non-planar slicing
* Support for more advanced Gcode words and macros
* Better multi-object support
* Better multi-material support
* Edit model in-slicer
* Export support structures along with model
* FEM simulation

## Dependencies
* [OpenCasCade](https://www.opencascade.com/)
*

## Supported CNC software:
* [LinuxCNC](https://github.com/LinuxCNC/linuxcnc)
* [Machinekit](http://www.machinekit.io/)
* [Redeem](http://wiki.thing-printer.com/index.php?title=Redeem)

## Build Instructions
```
git clone && cd
mkdir build && cd build
cmake ..
make
sudo make install
```
