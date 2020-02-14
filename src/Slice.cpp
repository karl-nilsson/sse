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

#include <sse/Slice.hpp>

namespace sse {

  // FIXME: figure out what to do with filename field of Object
Slice::Slice(TopoDS_Shape &s)
    : Object(s) {

  faces = std::vector<std::unique_ptr<Face>>();

  // search the slice for faces parallel and coincident with slicing plane
  // TODO: optimize! this is extremely inefficient
  // TODO: may be possible to use BRepAdaptor_* instead of GeomLProp_*
  // TODO: revamp for nonplanar slicing
  int i = 0;
  for (TopExp_Explorer exp(s, TopAbs_FACE); exp.More(); exp.Next()) {
    spdlog::debug(i);
    // cast to the correct type
    auto f = TopoDS::Face(exp.Value());
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
    // if normal isn't the same (opposite) as slicing plane, short-circuit
    // use the bounding box to
    // TODO: verify floating point equality, low epsilon
    if (gp::DZ().IsOpposite(props.Normal(), 0.01) &&
        fabs(props.Value().Z() - get_bound_box().CornerMin().Z()) < pow(10, -6)) {
      // add face to the list of faces for the slice
      faces.push_back(std::make_unique<Face>(f));
    }
  }
}

void Slice::generate_shells(int num, double width) {

}

void Slice::generate_infill(double percent, double angle, double line_width) {
  // for rectilinear infill, the infill% = num lines * line width / face width
  int num_lines = percent * width() / line_width;
  // get the bounding box's underlying points
  Standard_Real xmin, xmax, ymin, ymax;
  get_footprint().Get(xmin, ymin, xmax, ymax);
  // calculate offset between lines;
  double offset = width() / num_lines;
  for(int i = 0; i < num_lines; ++i) {
      // generate line
      // x1 = xmin + (i * offset)
      // y1 = ymin
      // x2 = xmin + (y / tan(angle))
    }
}

bool Slice::operator<(const Slice& rhs) const {
  // compare lowest Z coordinate of both bounding boxes
  return get_bound_box().CornerMin().Z() < rhs.get_bound_box().CornerMin().Z();
}

} // namespace sse
