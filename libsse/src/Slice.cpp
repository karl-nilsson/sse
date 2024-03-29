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
 * @file Slice.cpp
 *
 * @author Karl Nilsson
 *
 *
 * Required reading:
 * http://www.lee-mac.com/bulgeconversion.html
 * https://www.afralisp.net/archive/lisp/Bulges1.htm
 *
 */

// system headers
#include <algorithm>
#include <cmath>
#include <map>
#include <string>
// OCCT headers
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRep_Tool.hxx>
#include <GeomLProp_SLProps.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <ShapeAnalysis.hxx>
#include <ShapeAnalysis_Wire.hxx>
#include <Standard_Version.hxx>
#include <StdFail_NotDone.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <gp_Lin.hxx>
// external headers
#include <spdlog/spdlog.h>
// project headers
#include <sse/Slice.hpp>

#include "cavc/polylinecombine.hpp"
#include "cavc/polylineoffsetislands.hpp"

using namespace std::string_literals;

static cavc::Polyline<double> flatten(const gp_Circ &curve,
                                      const Geom_TrimmedCurve &trimmed_curve) {
  cavc::Polyline<double> result;

  auto radius = curve.Radius();
  auto center_point = curve.Location();
  auto angle =
      abs(trimmed_curve.LastParameter() - trimmed_curve.FirstParameter());
  auto first = trimmed_curve.FirstParameter();
  auto last = trimmed_curve.LastParameter();
  // TODO: verify
  auto clockwise =
      trimmed_curve.FirstParameter() < trimmed_curve.LastParameter() ? 1 : -1;

  auto start_point = trimmed_curve.StartPoint();

  // anything over a semicircle must be split
  if (angle > M_PI) {
    result.addVertex(start_point.X(), start_point.Y(), clockwise);
    // move start point
    start_point = trimmed_curve.Value(trimmed_curve.FirstParameter() +
                                      (clockwise * M_PI));
    // recompute angle
    angle -= M_PI;
  }

  double bulge = clockwise * tan(angle / 4);

  result.addVertex(start_point.X(), start_point.Y(), bulge);

  return result;
}

static cavc::Polyline<double> flatten(const gp_Elips &curve,
                                      const Geom_TrimmedCurve &trimmed_curve) {
  cavc::Polyline<double> result;
  // flatten the ellipse into arcs
  auto major_radius = curve.MajorRadius();
  auto minor_radius = curve.MinorRadius();
  auto center_point = curve.Location();
  auto angle = trimmed_curve.LastParameter() - trimmed_curve.FirstParameter();
  // TODO: verify
  auto clockwise =
      trimmed_curve.FirstParameter() < trimmed_curve.LastParameter() ? 1 : -1;
  auto bulge = 0.0;

  result.addVertex(trimmed_curve.StartPoint().X(),
                   trimmed_curve.StartPoint().Y(), 0);
  return {};
}

static cavc::Polyline<double> flatten(Geom_BSplineCurve &curve,
                                      const Geom_TrimmedCurve &trimmed_curve) {

  cavc::Polyline<double> result;
  result.addVertex(trimmed_curve.StartPoint().X(),
                   trimmed_curve.StartPoint().Y(), 0);
  return {};
}

static cavc::Polyline<double> flatten(Geom_BezierCurve &curve,
                                      const Geom_TrimmedCurve &trimmed_curve) {
  cavc::Polyline<double> result;

  curve.NbPoles();
  curve.Degree();

  result.addVertex(trimmed_curve.StartPoint().X(),
                   trimmed_curve.StartPoint().Y(), 0);

  return result;
}

static cavc::Polyline<double> process_wire(TopoDS_Wire w) {
  cavc::Polyline<double> result;
// reserve enough space for the child edges
// TODO: consider reserving a multiple of known children
#if OCC_VERSION_HEX >= 0x070400
  result.vertexes().reserve(w.NbChildren());
#endif

  // we only care about closed wires
  result.isClosed() = true;

  if (!w.Closed()) {
    spdlog::trace("Slice: Wire is open, skipping");
    return result;
  }

  // reverse wire if necessary
  if (w.Orientation() == TopAbs_REVERSED) {
    spdlog::trace("Slice: Reversing wire");
    w.Reverse();
  }

  // loop over edges in wire
  auto exp = BRepTools_WireExplorer(w);

  while (exp.More()) {
    // get underlying curve
    auto curve = BRepAdaptor_Curve(exp.Current());
    Standard_Real first_parameter, last_parameter;
    auto c2 = BRep_Tool::Curve(exp.Current(), first_parameter, last_parameter);
    // trim the curve
    auto trim = Geom_TrimmedCurve(c2, first_parameter, last_parameter);

    // if the geometry is reversed from the topology, reverse the former
    if (exp.Current().Orientation() == TopAbs_REVERSED) {
      spdlog::trace("Slice: Geometry of trimmed curve is opposite of Topology, "
                    "reversing geom_curve");
      trim.Reverse();
    }

    auto first = trim.FirstParameter();
    auto last = trim.LastParameter();

    auto start_point = trim.StartPoint();

    switch (curve.GetType()) {
    case GeomAbs_Line: {
      result.addVertex(start_point.X(), start_point.Y(), 0);
      break;
    }

    case GeomAbs_Circle: {
      auto z = flatten(curve.Circle(), trim);
      // append result to
      result.vertexes().insert(result.vertexes().end(), z.vertexes().begin(),
                               z.vertexes().end());
      break;
    }

    case GeomAbs_Ellipse: {
      auto z = flatten(curve.Ellipse(), trim);

      break;
    }
    case GeomAbs_BezierCurve: {
      // flatten the bezier into lines and arcs
      curve.Bezier();
      break;
    }
    case GeomAbs_BSplineCurve:
      result.addVertex(start_point.X(), start_point.Y(), 0);
      break;
    case GeomAbs_Hyperbola:
      result.addVertex(start_point.X(), start_point.Y(), 0);
      break;
    case GeomAbs_Parabola:
      result.addVertex(start_point.X(), start_point.Y(), 0);
      break;
    case GeomAbs_OffsetCurve:
      result.addVertex(start_point.X(), start_point.Y(), 0);
      break;
    case GeomAbs_OtherCurve:
      result.addVertex(start_point.X(), start_point.Y(), 0);
      break;
    default:
      // something is wrong, throw an error

      break;
    }

    exp.Next();
  }

  return result;
}

/**
 * @brief polyline_gcode Convert a polyline into gcode commands
 *
 * Traverse the polyline, converting each segment to a gcode command.
 *
 * @param pline Polyline to convert
 * @return String containing list of gcode commands
 */
static std::string polyline_gcode(const cavc::Polyline<double> &pline, double filament_diameter,
                                  double extrusion_width, double layer_height, double extrusion_multiplier) {

  // TODO: configurable extruder
  // either pull in setting here, or do a second format pass elsewhere
  // i.e. "{{extruder}}{:.3f}"

  if (pline.size() == 0) {
    return ""s;
  }

  std::string result = "G92 E0\n"s;

  double extrusion_total = 0.0;


  // TODO: profile preallocation length
  // TODO: refine
  const int gcode_string_length = fmt::format("G2 X{:.6f} Y{:.6f} I{:.6f} J{:.6f} E{:.6f} F1000\n",
                                              1.0, 1.0, 1.0, 1.0, 1.0).size() + 100;
  result.reserve(pline.size() * gcode_string_length);

  // travel to start of polyline
  // n.b. for closed polylines, this travels to the *last* point in polyline, because the visit function doesn't close the loop
  // there's probably a much better way to do this
  if(pline.isClosed()) {
    result += fmt::format("G0 X{:.6f} Y{:.6f}\n", pline.lastVertex().x(), pline.lastVertex().y());
  } else {
    result += fmt::format("G0 X{:.6f} Y{:.6f}\n", pline[0].x(), pline[0].y());
  }

  // multiply this by segment length to determine extrusion value
  // https://3dprinting.stackexchange.com/questions/6289/how-is-the-e-argument-calculated-for-a-given-g1-command
  auto extrusion_ratio = extrusion_multiplier * layer_height * 4 / (M_PI * filament_diameter);

  auto segment_to_gcode = [&](std::size_t i, std::size_t j) {
    auto &source = pline[i];
    auto &destination = pline[j];

    extrusion_total += (cavc::segLength(source, destination) * extrusion_ratio);

    // short-circuit for straight segment
    if (source.bulgeIsZero()) {
      result += fmt::format("G1 X{:.6f} Y{:.6f} E{:.6f} F1000\n", destination.x(),
                            destination.y(), extrusion_total);
      return true;
    }

    auto [radius, center] = cavc::arcRadiusAndCenter(source, destination);

    // negative bulge = clockwise
    // center is an absolute location (point), G(2|3) needs offsets relative to
    // starting point
    result += fmt::format("G{:s} X{:.6f} Y{:.6f} I{:.6f} J{:.6f} E{:.6f} F1000\n",
                          source.bulgeIsNeg() ? "2" : "3",
                          destination.x(),
                          destination.y(), center.x() - source.x(),
                          center.y() - source.y(),
                          extrusion_total);

    // visit all segments
    return true;
  };

  pline.visitSegIndices(segment_to_gcode);

  return result;
}

namespace sse {

Slice::Slice(const Object *parent, TopoDS_Face face, double thickness)
        : parent{parent}, face{face}, thickness{thickness} {
  if (this->face.IsNull()) {
    spdlog::error("Empty face passed to Slice");
    throw std::invalid_argument("Empty face");
  }

  wires = TopTools_ListOfShape();

  // get the parametric boundaries of the face
  Standard_Real umin, umax, vmin, vmax;
  BRepTools::UVBounds(face, umin, umax, vmin, vmax);
  // get the properties of surface, in order to get a point on the surface,
  // and the normal at said point
  // TODO: choose better U,V values
  auto props = GeomLProp_SLProps(BRep_Tool::Surface(face),
                                 (umin + umax) / 2, (vmin + vmax) / 2, 1, 1e-6);

  z = props.Value().Z();
}

void Slice::generate_shells(const int num_shells, const double line_width, const double overlap) {
  if(num_shells < 0) {
    spdlog::error("Slice: cannot generate less that zero shells");
    throw std::invalid_argument("Slice: cannot generate less than zero shells");
  }

  if(num_shells == 0) {
    spdlog::warn("Slice: generating zero offsets");
    return;
  }

  cavc::OffsetLoopSet<double> loopset;
  loopset.cwLoops.reserve(2);

#if OCC_VERSION_HEX >= 0x070400
  loopset.ccwLoops.reserve(face.NbChildren());
#endif

  auto outer_wire = ShapeAnalysis::OuterWire(face);

  for (auto exp = TopExp_Explorer(face, TopAbs_WIRE); exp.More(); exp.Next()) {
    auto r = process_wire(TopoDS::Wire(exp.Current()));

    // outer wire is counter-clockwise, islands are clockwise
    if (outer_wire.IsSame(exp.Current())) {
      loopset.ccwLoops.push_back({0, r, cavc::createApproxSpatialIndex(r)});
    } else {
      loopset.cwLoops.push_back({0, r, cavc::createApproxSpatialIndex(r)});
    }
  }

  cavc::ParallelOffsetIslands<double> alg;

  // n.b. first shell must be 1/2 * line_width offset, or else extrusion will inflate exterior dimensions
  for (int i = 0; i < num_shells; ++i) {
    auto result = alg.compute(loopset, (i + 0.5) * line_width);
    this->shells.emplace_back(result);
  }

  // generate innermost offset, used for clipping infill
  const auto innermost_offset = (num_shells + 1 + overlap) * line_width;

  // TODO: this is very ugly. can't construct Shell for some reason
  const auto tmp = alg.compute(loopset, innermost_offset);
  auto &outer = innermost_shell.outer;
  auto &z = tmp.ccwLoops.front().polyline;

  innermost_shell.outer = z;

  /*
  std::swap(outer, z);
  std::swap(outer, tmp.ccwLoops.front().polyline);
  for(auto &loopset: tmp.cwLoops) {
    innermost_shell.islands.push_back(std::move(loopset.polyline));
  }
  */

}

void Slice::generate_infill(cavc::Polyline<double> infill_pattern) {

  if (shells.empty()) {
    spdlog::error("Slice: cannot generate infill before offsetting");
    return;
  }

  // intersect with innermost polyline
  auto infill = cavc::intersect_open_polyline(innermost_shell.outer, infill_pattern);
  
  auto &result = infill.remaining;

  // cut islands (clockwise loops)
  // TODO: optimize. this is extremely inefficient
  // TODO: cavc hasn't implemented exclude operation on open polylines
  /*
  for(const auto &pline: innermost_shell.islands) {

    std::vector<cavc::Polyline<double>> tmp;
    tmp.reserve(result.size());

    for (auto &i : result) {
      auto a = cavc::combinePolylines(i, pline,
                                      cavc::PlineCombineMode::Exclude);
      tmp.insert(tmp.end(), a.remaining.begin(), a.remaining.end());
    }
    result = tmp;
  }
  */

  infill_polylines = result;
}


std::string Slice::gcode(double filament_diameter, double extrusion_width, double extrusion_multiplier) const {
  std::string result;

  if(shells.empty()) {
    spdlog::warn("Slice: generating infill with no shells or infill");
    return result;
  }

  // TODO: profile whether 1KB is a good choice for preallocation
  result.reserve(1000);

  // shells first
  for(auto shell = shells.cbegin(); shell != shells.cend(); ++shell) {
    result += ";TYPE:WALL-"s;
    result += (shell == shells.cbegin()) ? "OUTER\n"s : "INNER\n"s;

    result += polyline_gcode(shell->outer, filament_diameter, extrusion_width, this->thickness, extrusion_multiplier);

    for(const auto &pline: shell->islands) {
      result += polyline_gcode(pline, filament_diameter, extrusion_width, this->thickness, extrusion_multiplier);
    }
  }

  // infill second
  // TODO: sort infill segments
  if(infill_polylines.size() > 0) {
    result += ";TYPE:FILL\n"s;
  }
  for (const auto &pline : infill_polylines) {
    result += polyline_gcode(pline, filament_diameter, extrusion_width, this->thickness, extrusion_multiplier);
  }

  return result;
}

} // namespace sse
