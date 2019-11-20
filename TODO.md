TODO

- Add fmt library
- get toml lib working

Features:
- STEP assembly import
- generate support material
- manual support material placement/removal
- save/export support as a STEP child object
- logging (verbosity levels)
- export GCode as structured data, in addition to raw text

Refinement:
- charconv output for gcode coordinates
- Performance analysis: OSD_PerfMeter
- TBB vs OpenMP
- GCode pattern words
- Allow build volume to be represented as a 3d shell/solid (allows for hangprinter)
- Eval/use OCCT OCAF
- Adapter pattern
 + OCCT - string representation/debugging
 + OCCT - GCODE
