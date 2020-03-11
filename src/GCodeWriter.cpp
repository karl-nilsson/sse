/**
 * StepSlicerEngine
 * Copyright (C) 2020 Karl Nilsson
 *
 * This program is free software: you can redistribute it and/or modify
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @brief
 * @author Karl Nilsson
 *
 *
 */

#include <sse/GCodeWriter.hpp>

GCodeWriter::GCodeWriter() : config(sse::Settings::getInstance()) {
  data = std::string();
  // reserve a large buffer upfront
  data.reserve(INITIAL_GCODE_SIZE);
}

void GCodeWriter::create_header() {
  // a list of settings to include in the comment header
  auto settings_list = std::vector{"printer name", "layer_height"};

  // get the current date and time
  auto now =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  // first line of gcode program contains meta-information
  add_comment(
      fmt::format("Sliced by StepSlicerEngine on {0}", std::ctime(&now)));

  // FIXME
  for (auto s : settings_list) {
    // format settings: "key = value"
    // 10 digit precision, force inlining
    add_comment(toml::format(toml::find(config.config, s), 0, 10, true, true));
  }
}

void GCodeWriter::add_rapid(double x, double y, double z) {
  // get settings
  auto feedrate = config.get_setting<uint>("rapid");
  std::string move = fmt::format("G0 X{} Y{} Z{} F{};\n", x, y, z, feedrate);
  data.append(move);
}

void GCodeWriter::add_line(GeomAdaptor_Curve c) {
  // get settings
  auto feedrate = config.get_setting<uint>("E0.print_speed");
  auto a = c.Value(c.FirstParameter());
  double x, y, z;
  a.Coord(x, y, z);

  auto b = c.Value(c.LastParameter());
  b.Coord(x, y, z);

  // calculate line distance
  auto dist = a.Distance(b);

  double distance = fabs(c.FirstParameter() - c.LastParameter());

  std::string move =
      fmt::format("G1 X{} Y{} Z{} E{} F{}\n;", x, y, z, distance, feedrate);
  data.append(move);
}

void GCodeWriter::add_arc(GeomAdaptor_Curve c) {
  // TODO: figure out whether CW or CCW
  std::string move = fmt::format("G2 X{} Y{} Z{} I{} J{} P{} E{}\n");
  data.append(move);
}

void GCodeWriter::add_nurbs() {}

void GCodeWriter::add_bezier(Geom_BezierCurve b) {}

void GCodeWriter::add_bslpine(Geom_BSplineCurve b) {}

void GCodeWriter::add_wire(TopoDS_Wire w) {
  // explore the wire
  auto a = BRep_Tool();
  for (BRepTools_WireExplorer we(w); we.More(); we.Next()) {
      // get current edge
      auto& edge = we.Current();
      Standard_Real u_min, u_max;
      // get unbounded curve
      auto curve = std::make_unique<Geom_Curve>(BRep_Tool::Curve(edge, u_min, u_max));
      // trim curve
      auto z = std::make_unique<Geom_TrimmedCurve>(curve, u_min, u_max);
      z->IsKind();
      spdlog::debug("{}", z->DynamicType());
      STANDARD_TYPE(Geom_Line);
      // process curve
      add_segment(curve);
  }
}

std::string GCodeWriter::add_segment(Geom_Curve c) {
  // get settings
  auto feedrate = config.get_setting<uint>("E0.print_speed");
  auto a = c.Value(c.FirstParameter());
  double x, y, z;
  a.Coord(x, y, z);

  auto b = c.Value(c.LastParameter());
  b.Coord(x, y, z);

  auto dist = a.Distance(b);

  double distance = fabs(c.FirstParameter() - c.LastParameter());

  switch (c.GetType()) {
  case GeomAbs_Line:
    return add_line(c);
  case GeomAbs_Circle:
    return add_arc(c);
  case GeomAbs_Ellipse:
    break;
/*
  case GeomAbs_NonUniform:
    return "G5.2";
    break;
*/
  case GeomAbs_BezierCurve:
    // split curve
    return add_bezier(c);
  case GeomAbs_BSplineCurve:
    // split spline
    return add_bspline(c);
  default:
    throw new std::runtime_error("GCodeWriter: Invalid segment type");
    break;
  }
}

void GCodeWriter::retract(double distance) {
  auto retraction_speed = config.get_setting<uint>("E0.retraction_speed");
  std::string move = fmt::format("G92 E0;\nG1 E{} F{};\nG92 E0;\n", distance,
                                 retraction_speed);
  data.append(move);
}
