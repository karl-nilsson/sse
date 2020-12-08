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
 * @file
 *
 * @author Karl Nilsson
 */
// system headers
#include <cmath>
#include <algorithm>
#include <map>
// OCCT headers
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <StdFail_NotDone.hxx>
#include <GeomLProp_SLProps.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <gp_Lin.hxx>
#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <Geom_BezierCurve.hxx>
// external headers
#include <spdlog/spdlog.h>
// project headers
#include <sse/Object.hpp>
#include <sse/Slice.hpp>

#include "cavc/polylineoffset.hpp"

namespace sse {

// FIXME: figure out what to do with filename field of Object
Slice::Slice(TopoDS_Shape &s) : Object(s) {
  // regenerate bounding box, optimized with no gap
  generate_bounds(true, 0.0);

  faces = TopTools_HSequenceOfShape();
  wires = TopTools_ListOfShape();


  auto faces_map = std::map<int,TopTools_HSequenceOfShape>();

  // search the slice for faces parallel and coincident with slicing plane
  // TODO: optimize! this is extremely inefficient
  // TODO: may be possible to use BRepAdaptor_* instead of GeomLProp_*
  // TODO: revamp for nonplanar slicing
  for (TopExp_Explorer exp(s, TopAbs_FACE); exp.More(); exp.Next()) {
    // cast to the correct type
    auto f = TopoDS::Face(exp.Current());
    // get underlying geometry
    auto b = BRepAdaptor_Surface(f);
    // if face is not a plane, short-circuit
    if (b.GetType() != GeomAbs_Plane) {
      continue;
    }

    // get the parametric boundaries of the face
    Standard_Real umin, umax, vmin, vmax;
    BRepTools::UVBounds(f, umin, umax, vmin, vmax);
    // get the properties of surface, in order to get a point on the surface,
    // and the normal at said point
    // TODO: choose better U,V values
    auto props = GeomLProp_SLProps(BRep_Tool::Surface(f), (umin + umax) / 2,
                                   (vmin + vmax) / 2, 1, 1e-6);
    auto normal = props.Normal();

    // take orientation into account
    if (f.Orientation() == TopAbs_REVERSED) {
      normal.Reverse();
    }

    // if normal isn't the same (opposite) as slicing plane, short-circuit
    // TODO: verify floating point equality, low epsilon
    spdlog::debug("Face z: {:.3f}  Bounds z: {:.3f}", props.Value().Z(), get_bound_box().CornerMin().Z());
    // only need downward-facing planes
    if (gp::DZ().IsOpposite(normal, 0.01)) {
      // add face to the list of faces for the slice
      spdlog::debug("Sice: adding face {:.3f}", props.Value().Z());
      // store faces by z-value, round to avoid problems
      // TODO: figure out better epsilon/round value
      long long rounded_z = floor(props.Value().Z() * 100000);
      faces_map[rounded_z].Append(f);
    }
  }

  // the first (lowest) face(s) are what we want
  faces = faces_map.begin()->second;

}

cavc::Polyline<double> process_wire(TopoDS_Wire w) {
  cavc::Polyline<double> result;
  // we only care about closed wires
  result.isClosed() = true;

  if(!w.Closed()) {
    spdlog::debug("Wire is open, skipping");
    return result;
  }

  // reverse wire if necessary
  if(w.Orientation() == TopAbs_REVERSED) {
    spdlog::debug("Reversing wire");
    w.Reverse();
  }

  // loop over edges in wire
  auto exp = BRepTools_WireExplorer(w);

  while (exp.More()) {
    auto curve = BRepAdaptor_Curve(exp.Current());
    Standard_Real first, last;
    auto c2 = BRep_Tool::Curve(exp.Current(), first, last);
    // trim the curve
    auto trim = Geom_TrimmedCurve(c2, first, last);

    // if the geometry is reversed from the topology, reverse the former
    auto start_vertex = BRepBuilderAPI_MakeVertex(trim.StartPoint());
    if(exp.Current().Orientation() == TopAbs_REVERSED) {
      spdlog::debug("Geometry of trimmed curve is opposite of Topology, reversing geom_curve");
      trim.Reverse();
    }

    auto start_point = trim.StartPoint();

    // for now, assume everything is a line
    result.addVertex(start_point.X(), start_point.Y(), 0);


    /*
    switch (curve.GetType()) {
    case GeomAbs_Line: {
      result.addVertex(start_point.X(), start_point.Y(), 0);
      break;
    }

    case GeomAbs_Circle: {
      auto circle = curve.Circle();
      auto radius = circle.Radius();
      auto center_point = circle.Location();
      auto angle = abs(last - first);
      auto clockwise = last > first;
      auto bulge = 0;

      // anything over a semicircle must be split
      if (angle > M_PI) {
        auto end_point = curve.FirstParameter() + M_PI;
        // TODO: figure out +/- bulge value
        result.addVertex(start_point.X(), start_point.Y(), 1);

        // move to the start of the next arc
        first += M_PI;
        // last parameter is guaranteed to be larger than the first
        angle = last - first;
        start_point = trim.Value(first);
      }

      bulge = tan(angle / 4);

      result.addVertex(start_point.X(), start_point.Y(), bulge);
      break;
    }

    case GeomAbs_Ellipse: {
      // flatten the ellipse into arcs
      auto ellipse = curve.Ellipse();
      auto major_radius = ellipse.MajorRadius();
      auto minor_radius = ellipse.MinorRadius();
      auto center_point = ellipse.Location();
      auto angle = curve.LastParameter();
      auto bulge = 0;

      for (int i = 0; i < 1; ++i) {
        result.addVertex(start_point.X(), start_point.Y(), bulge);
      }

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
    }*/

    exp.Next();
  }

  return result;
}

// TODO: better API, i.e. list of offset dimensions
void Slice::generate_shells(int num, double width) {

  // loop over all faces
  for (const auto &f : faces) {
    // cast from TopoDS_Shape to TopoDS_Face
    const TopoDS_Face &face = TopoDS::Face(f);
    // process wires
    for(auto exp = TopExp_Explorer(face, TopAbs_WIRE); exp.More(); exp.Next()) {
      process_wire(TopoDS::Wire(exp.Current()));

      //exp.Current().DumpJson(std::cout);


    }

  }
}

void Slice::generate_infill(double percent, double angle, double line_width) {
  // for rectilinear infill, the infill% = num lines * line width / face width
  int num_lines = percent * width() / line_width;
  // get the bounding box's underlying points
  Standard_Real xmin, xmax, ymin, ymax;
  get_footprint().Get(xmin, ymin, xmax, ymax);
  // calculate offset between lines;
  double offset = width() / num_lines;
  for (int i = 0; i < num_lines; ++i) {
    // generate line
    // x1 = xmin + (i * offset)
    // y1 = ymin
    // x2 = xmin + (y / tan(angle))
  }
}

bool Slice::operator<(const Slice &rhs) const {
  // compare lowest Z coordinate of both bounding boxes
  return get_bound_box().CornerMin().Z() <= rhs.get_bound_box().CornerMin().Z();
}

} // namespace sse
