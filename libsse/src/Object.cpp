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
 * @brief
 * @author
 *
 * @bug TODO: consider changing transform to TopoDS_Shape.Location()
 */

#include <sse/Object.hpp>

#include <BRepBuilderAPI_Copy.hxx>

namespace sse {

Object::Object(TopoDS_Shape &shape, const std::string &fname) : shape(std::make_unique<TopoDS_Shape>(shape)), filename(fname) {
  spdlog::debug("Object: Initializing object with shape");
  // calculate the axis-aligned bounding box
  bounding_box = Bnd_Box();
  footprint = Bnd_Box2d();
  generate_bounds();
}

Object::Object(const Object& o): filename(o.filename) {
  spdlog::debug("Object: copying object");
  // copy underlying shape from unique_ptr
  BRepBuilderAPI_Copy b{*o.shape};
  shape = std::make_unique<TopoDS_Shape>(b.Shape());
  // calculate AABB
  bounding_box = Bnd_Box();
  footprint = Bnd_Box2d();
  generate_bounds();
}

Object& Object::operator =(const Object& o) {
  spdlog::debug("Object: copying object");
  filename = o.filename;
  // copy underlying shape from unique_ptr
  BRepBuilderAPI_Copy b{*o.shape};
  shape = std::make_unique<TopoDS_Shape>(b.Shape());
  // calculate AABB
  bounding_box = Bnd_Box();
  footprint = Bnd_Box2d();
  generate_bounds();

  return *this;
}

void Object::generate_bounds(bool optimal, double gap) {
  spdlog::debug("Object: Generating bounding box");
  // clear bounding box
  bounding_box.SetVoid();
  // reset gap
  // TODO: configurable gap
  bounding_box.SetGap(gap);
  // create bounding box
  // TODO: test perf of BRepBndLib::AddOptimal
  if (optimal) {
    BRepBndLib::AddOptimal(*shape, bounding_box);
  } else {
    BRepBndLib::Add(*shape, bounding_box);
  }

  spdlog::debug("Object: generating footprint");
  // clear footprint
  footprint.SetVoid();
  // add corner points to footprint
  // unfortunately, I can't find an elegant way to convert gp_Pnt to gp_Pnt2d
  try {
    footprint.Add(
        gp_Pnt2d(bounding_box.CornerMin().X(), bounding_box.CornerMin().Y()));
    footprint.Add(
        gp_Pnt2d(bounding_box.CornerMax().X(), bounding_box.CornerMax().Y()));
  }  catch (Standard_ConstructionError &e) {
    spdlog::error(e.GetMessageString());
  }
}

void Object::lay_flat(const TopoDS_Face &face) {
  spdlog::info("Object: laying object flat");
  // get the u,v bounds of selected surface
  Standard_Real umin, umax, vmin, vmax;
  BRepTools::UVBounds(face, umin, umax, vmin, vmax);
  // get the normal of the selected face at (u,v)
  // TODO: select the correct parameters (u,v)
  // currently chooses the "average" parameters
  auto props = GeomLProp_SLProps(BRep_Tool::Surface(face), (umin + umax) / 2,
                                 (vmin + vmax) / 2, 1, 1e-6);
  // normal undefined, error
  // TODO: raise error?
  if (!props.IsNormalDefined()) {
    return;
  }

  auto normal = props.Normal();
  // TODO: better log messages
  spdlog::debug("Face normal: {:f},{:f},{:f}", normal.X(), normal.Y(),
                normal.Z());
  // get the coordinate of the face at (u,v)
  auto point = props.Value();
  // if the face was reversed, reverse the normal
  // TODO: this may be unnecessary
  if (face.Orientation() == TopAbs_REVERSED) {
    normal.Reverse();
  }

  // if normals aren't already equal, rotate until they are
  // TODO: better decision regarding tolerance
  if (!normal.IsOpposite(gp::DZ(), 0.0001)) {
    // the axis of rotation is the cross product of the two vectors
    auto unit_vector = normal.Crossed(gp::DZ());
    spdlog::debug("Axis: {:f},{:f},{:f}");
    spdlog::debug("Angle: {:f}°");
    // rotate the object, so that the specified normal is opposite the +Z unit
    // vector
    // TODO: verify center point is correct/good idea
    // TODO: verify angle calc correctness
    rotate(gp_Ax1(center_point(), unit_vector), normal.Angle(gp::DZ()) + M_PI);

    // if rotation happened, recalculate coordinate
    // TODO: make more elegant
    point = GeomLProp_SLProps(BRep_Tool::Surface(face), (umin + umax) / 2,
                              (vmin + vmax) / 2, 1, 1e-6)
                .Value();
  }

  // move the shape so that the face is touching the XY plane
  translate(0, 0, -1 * point.Z());
}

gp_Pnt Object::center_point() const {
  auto min = bounding_box.CornerMin();
  auto max = bounding_box.CornerMax();
  return gp_Pnt((min.X() + max.X()) / 2,
                (min.Y() + max.Y()) / 2,
                (min.Z() + max.Z()) / 2);
}

void Object::mirror(gp_Ax2 mirror_plane) {
  spdlog::debug("Object: Mirror: ");
  auto mirror = gp_Trsf();
  mirror.SetMirror(mirror_plane);
  transform(mirror);
}

void Object::rotate(const gp_Ax1 axis, const double angle) {
  spdlog::debug("Object: Rotating {}°", angle);
  auto rotate = gp_Trsf();
  rotate.SetRotation(axis, angle * M_PI / 180);
  transform(rotate);
}

void Object::translate(const double x, const double y, const double z) {
  translate(gp_Vec(x, y, z));
}

void Object::translate(const gp_Vec v) {
  spdlog::debug("Object: Translating vector: {},{},{}", static_cast<double>(v.X()),
                static_cast<double>(v.Y()), static_cast<double>(v.Z()));
  auto translate = gp_Trsf();
  translate.SetTranslation(v);
  transform(translate);
}

void Object::translate(const gp_Pnt destination) {
  spdlog::debug("Object: Translating to ({},{},{})", static_cast<double>(destination.X()),
                static_cast<double>(destination.Y()), static_cast<double>(destination.Z()));
  auto translate = gp_Trsf();
  translate.SetTranslation(bounding_box.CornerMin(), destination);
  transform(translate);
  // translate bounding box as well
  // TODO: does this leak?
  bounding_box = bounding_box.Transformed(translate);
  footprint = footprint.Transformed(translate);
}

void Object::scale(const double x, const double y, const double z) {
  spdlog::debug("Object: Scaling: ");
  // TODO: get working
  auto v = gp_Vec(x, y, z);
  auto scale = gp_Trsf();
  transform(scale);
}

void Object::transform(const gp_Trsf transform) {
  try {
    auto s = BRepBuilderAPI_Transform(*shape, transform).Shape();
    shape = std::make_unique<TopoDS_Shape>(s);
  } catch (const StdFail_NotDone &e) {
    spdlog::error(e.GetMessageString());
  }
}

double Object::get_volume() const {
  GProp_GProps volume;
  BRepGProp::VolumeProperties(*shape, volume);
  return volume.Mass();
}

} // namespace sse
