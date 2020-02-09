TODO

Features:
- STEP assembly import
 - Per-child print settings
- generate support material
- manual support material placement/removal
- save/export supports as a STEP child object
- export GCode as structured data, in addition to raw text

Refinement:
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
- Eval/use OCCT OCAF
- Performance
  - analysis: OSD_PerfMeter
  - Parallelization: TBB vs OpenMP

Meta:
- CI
 - Travis: waiting on ubuntu 20.04 for recent OCCT packages
- cmake
 - clang-format
 - clang-tidy (cppcoreguidelines)
 - codespell
 - doxygen
