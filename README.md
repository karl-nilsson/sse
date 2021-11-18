# STEP Slicer Engine
[![Build Status](https://github.com/karl-nilsson/sse/actions/workflows/build.yml/badge.svg)](https://github.com/karl-nilsson/sse/actions/workflows/build.yml)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/3ed52535476d453f97456e77e79612c2)](https://app.codacy.com/manual/karl-nilsson/sse?utm_source=github.com&utm_medium=referral&utm_content=karl-nilsson/sse&utm_campaign=Badge_Grade_Dashboard)
[![codecov](https://codecov.io/gh/karl-nilsson/sse/branch/master/graph/badge.svg?token=DOLTWBEKR9)](https://codecov.io/gh/karl-nilsson/sse)


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

## Dependencies
* [OpenCasCade](https://www.opencascade.com/) > 7.3.0
* [TBB](https://software.intel.com/content/www/us/en/develop/tools/threading-building-blocks.html)
* [Doxygen](http://doxygen.nl/)
* [Graphviz](https://graphviz.org/)

## Supported CNC software:
* [LinuxCNC](http://linuxcnc.org/)
* [Machinekit](https://www.machinekit.io/)
* [Redeem](http://wiki.thing-printer.com/index.php?title=Redeem)

## License
[AGPL](LICENSE)

## Build Instructions
```
git clone https://github.com/karl-nilsson/sse.git && cd sse
cmake -S . -B build
cmake --build build
cd build && ctest
```

