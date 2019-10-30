
#include <sse/GCodeWriter.hpp>

GCodeWriter::GCodeWriter() : config(Settings::getInstance()) {
  data = std::string();
  // reserve a large buffer upfront
  data.reserve(INITIAL_GCODE_SIZE);
}

void GCodeWriter::add_comment(std::string comment) { data.append(comment); }

void GCodeWriter::add_rapid(int x, int y, int z) {
  // get settings
  // double feedrate = config.get("rapid");
  int feedrate = 1000;
  data.append("G0 X" + x);
}

void GCodeWriter::add_segment(int x, int y, int z, int e, int f) {
  // get settings
  data.append("G1");
}

void GCodeWriter::add_arc(int x, int y, int i, int j, int e, int f) {}

void GCodeWriter::add_nurbs() {}

void GCodeWriter::add_bezier(Geom_BezierCurve b) {

}

void GCodeWriter::add_bslpine(Geom_BSplineCurve b) {

}

void GCodeWriter::add_wire(TopoDS_Wire w) {
  // explore the wire
  for (TopExp_Explorer e(w, TopAbs_EDGE); e.More(); e.Next()) {
    try {
      // cast to an edge, then convert to gcode
      auto edge = TopoDS::Edge(e.Current());


      TopoDS_Vertex y, z;
      TopExp::Vertices(edge, y, z);
      Standard_Real a, b;
      BRep_Tool::Curve(edge, a,b);


    } catch (Standard_TypeMismatch &e) {
      e.Print(std::cerr);
    }
  }
}
