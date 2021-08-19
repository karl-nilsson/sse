# TODO

## Features
- import STEP assemblies
- cascading/inheritance settings architecture
  - pseudo-parent as root, holds default/fallback settings
  - children inherit settings of parents, but can override
  - YAML supports inheritance, TOML doesn't
- Supports
 - generate support solids
   - tree supports
 - manual support material placement/removal
 - save/export supports as a STEP file
 - use offset shape to create gap between solid and supports (BRepOffsetAPI_MakeOffsetShape)
- GCode
  - export GCode as structured data, in addition to raw text

## Refinement
- link only necessary parts of OCCT
- charconv output for gcode coordinates
- define config TOML schema
- GCode pattern words
- better build plate/volume representation
  - handle non-rectangular build plates (delta)
  - handle non-prism build volumes (hangprinter)
- Print simulation
  - collision detection between effector and workpiece (print)
- High-level OCCT:
  - OCAF: https://dev.opencascade.org/doc/overview/html/occt_user_guides__ocaf.html
  - XDE: https://dev.opencascade.org/doc/overview/html/occt_user_guides__xde.html
  - TObj
  - XCAFPrs_AISObject
- Performance
  - analysis: OSD_PerfMeter
  - Parallelization: TBB vs OpenMP
- Convert cavc_polygons to OCCT wires?
  - easier to render? (gcode preview/debugging)
- Better/Consistent separation of concerns between library and application
  - boils down to how much state the library should maintain
  - should object modification (e.g. translation/rotation) be part of library or application?
  - should settings engine be part of library or application?

## Meta
- cmake
 - clang-format
 - clang-tidy (cppcoreguidelines)
 - codespell
