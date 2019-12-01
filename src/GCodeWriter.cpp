
#include <sse/GCodeWriter.hpp>

GCodeWriter::GCodeWriter() : config(sse::Settings::getInstance()) {
  data = std::string();
  // reserve a large buffer upfront
  data.reserve(INITIAL_GCODE_SIZE);

}

/**
 * @brief GCodeWriter::create_header Create the comment header for the GCode file
 */
void GCodeWriter::create_header() {
  // a list of settings to include in the comment header
  auto settings_list = std::vector{"printer name", "layer_height"};

  // get the current date and time
  auto now = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now());
  add_comment(fmt::format("Sliced by StepSlicerEngine on {0}", std::ctime(&now)));

  // FIXME
  for (auto s : settings_list) {
      // have toml format to "key = value"
      // 10 digit precision, force inlining
      add_comment(toml::format(toml::find(config.config, s), 0, 10, true, true));
  }
}

/**
 * @brief GCodeWriter::add_comment add a comment line to the GCode
 * @param comment
 */
void GCodeWriter::add_comment(std::string comment) { data.append(";" + comment); }

void GCodeWriter::add_rapid(double x, double y, double z) {
  // get settings
  auto feedrate = config.get_setting<uint>("rapid");
  std::string move = fmt::format("G0 X{} Y{} Z{} F{};\n", x, y, z, feedrate);
  //data.append("G0 X" + x);
}

void GCodeWriter::add_line(GeomAdaptor_Curve c) {
  // get settings
  auto feedrate = config.get_setting<uint>("E0.print_speed");
  auto a = c.Value(c.FirstParameter());
  double x, y,z;
  a.Coord(x, y, z);

  auto b = c.Value(c.LastParameter());
  b.Coord(x, y, z);

  auto dist = a.Distance(b);

  double distance = fabs(c.FirstParameter() - c.LastParameter());

  std::string move = fmt::format("G1 X{} Y{} Z{} E{} F{}\n;", x, y, z, distance, feedrate);
  data.append(move);
}

void GCodeWriter::add_arc(GeomAdaptor_Curve c) {
    // TODO: figure out whether CW or CCW
    std::string move = fmt::format("G2 X{} Y{} Z{} I{} J{} P{} E{}");
}

void GCodeWriter::add_nurbs() {}

void GCodeWriter::add_bezier(Geom_BezierCurve b) {

}

void GCodeWriter::add_bslpine(Geom_BSplineCurve b) {

}

void GCodeWriter::add_wire(TopoDS_Wire w) {
  // explore the wire
  auto a = BRep_Tool();
  for (TopExp_Explorer e(w, TopAbs_EDGE); e.More(); e.Next()) {
    try {
      // cast to an edge, then convert to gcode
      auto edge = TopoDS::Edge(e.Current());
      Standard_Real u,v;
      auto curve = GeomAdaptor_Curve(a.Curve(edge, u, v));

      TopoDS_Vertex y, z;
      TopExp::Vertices(edge, y, z);


    } catch (Standard_TypeMismatch &e) {
      e.Print(std::cerr);
    }
  }
}

std::string GCodeWriter::add_segment(GeomAdaptor_Curve c) {
    // get settings
    auto feedrate = config.get_setting<uint>("E0.print_speed");
    auto a = c.Value(c.FirstParameter());
    double x, y,z;
    a.Coord(x, y, z);

    auto b = c.Value(c.LastParameter());
    b.Coord(x, y, z);

    auto dist = a.Distance(b);

    double distance = fabs(c.FirstParameter() - c.LastParameter());

    switch(c.GetType()) {
        case GeomAbs_Line: return "G1 X{} Y{} Z{} E{} F{}"; break;
        case GeomAbs_Circle: return "G3" ; break;
        case GeomAbs_Ellipse: break;
//          case GeomAbs_NonUniform: break;
        case GeomAbs_BezierCurve: return "G4"; break;
        case GeomAbs_BSplineCurve: return "G5 "; break;
        default: break;
        }

}

void GCodeWriter::retract(double distance) {
    auto retraction_speed = config.get_setting<uint>("E0.retraction_speed");
    std::string move = fmt::format("G92 E0;\nG1 E{} F{};\nG92 E0;\n", distance, retraction_speed);

}
