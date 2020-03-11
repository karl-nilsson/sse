# TODO

## Features
- STEP assembly import
 - Per-child print settings
- Supports
 - generate support material
 - tree supports
 - manual support material placement/removal
 - save/export supports as a STEP child
- GCode
  - export GCode as structured data, in addition to raw text

## Refinement
- only link necessary parts of OCCT
- charconv output for gcode coordinates
- define config TOML schema
- refine clipper lib
  - cmake library build
  - separate repo
  - header only library?
  - better algo: DETC2014-34303
- GCode pattern words
- represent build volume as 3d shell (hangprinter)
- store effector as model for collision detection
- High-level OCCT:
  - OCAF: https://dev.opencascade.org/doc/overview/html/occt_user_guides__ocaf.html
  - XDE: https://dev.opencascade.org/doc/overview/html/occt_user_guides__xde.html
  - TObj
- Performance
  - analysis: OSD_PerfMeter
  - Parallelization: TBB vs OpenMP

## Meta
- CI
 - Travis: blocker: ubuntu 20.04 for OCCT v7.4
- cmake
 - clang-format
 - clang-tidy (cppcoreguidelines)
 - codespell
