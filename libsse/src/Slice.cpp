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

// OCCT headers
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <StdFail_NotDone.hxx>
#include <GeomLProp_SLProps.hxx>
// external headers
#include <spdlog/spdlog.h>
// project headers
#include <sse/Object.hpp>
#include <sse/Slice.hpp>

namespace sse {

// FIXME: figure out what to do with filename field of Object
Slice::Slice(TopoDS_Shape &s) : Object(s) {
  // regenerate bounding box, optimized with no gap
  generate_bounds(true, 0.0);

  faces = TopTools_HSequenceOfShape();
  wires = TopTools_ListOfShape();

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
    if (f.Orientation() == TopAbs_REVERSED) {
      normal.Reverse();
    }
    // if normal isn't the same (opposite) as slicing plane, short-circuit
    // TODO: verify floating point equality, low epsilon
    // spdlog::debug("Face z: {}  Bounds z: {}", props.Value().Z(), get_bound_box().CornerMin().Z());
    // std::cout << "Face z: " << props.Value().Z() << " Bounds z: " << get_bound_box().CornerMin().Z() << std::endl;
    // TODO: bounding box bottom face isn't the same as the slicing plane. Need
    // to pass in slicing plane to get the correct faces if
    // (gp::DZ().IsOpposite(props.Normal(), 0.01) && fabs(props.Value().Z() - get_bound_box().CornerMin().Z()) < pow(10, -6)) {
    if (gp::DZ().IsOpposite(normal, 0.01)) {
      // add face to the list of faces for the slice
      spdlog::debug("Sice: adding face");
      // faces.push_back(Face(f));
      faces.Append(f);
    }
  }
}

// TODO: better API, i.e. list of offset dimensions
void Slice::generate_shells(int num, double width) {

  // loop over all faces
  for (const auto &f : faces) {
    // cast from TopoDS_Shape to TopoDS_Face
    const TopoDS_Face &face = TopoDS::Face(f);
    // adding face automatically adds all wires
    auto b = BRepOffsetAPI_MakeOffset(face, GeomAbs_Arc);

    try {
      // for each offset
      for (int i = 1; i <= num; ++i) {
        // make the offset
        // TODO: allow for both outward and inward offsets
        b.Perform(-1 * i * width);
      }

    } catch (StdFail_NotDone &e) {
      spdlog::error("Slice: offset failure");
      // catch build error
      e.Print(std::cerr);
    }

    // get all the generated offsets
    // FIXME: currently returns empty list
    auto tmp = b.Generated(f);
    for (const auto &s : tmp) {
      spdlog::debug(TopAbs::ShapeTypeToString(s.ShapeType()));
    }
    wires.Append(tmp);
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
