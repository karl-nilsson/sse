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

#pragma once

#include <BRepBndLib.hxx>
#include <BRepBuilderAPI.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>

#include <BndLib.hxx>
#include <Bnd_Box.hxx>
#include <Bnd_Box2d.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <gp.hxx>
#include <gp_Pln.hxx>
#include <gp_Trsf.hxx>

#include <TCollection.hxx>
#include <TCollection_AsciiString.hxx>

#include <AIS_Shape.hxx>
#include <GeomLProp_SLProps.hxx>
#include <Standard_ConstructionError.hxx>
#include <StdFail_NotDone.hxx>

#include <math.h>

#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

namespace sse {

/**
 * @brief The Object class
 */
class Object {

public:
  /**
   * @brief Object
   * @param object
   */
  Object(const Object& object);

  /**
   * @brief Object
   * @param s
   */
  Object(TopoDS_Shape s);

  /**
   * @brief generate_bounds
   */
  void generate_bounds();

  /**
   * @brief Rotate and translate object so that one face is flat on the buildplate
   * @param face
   */
  void lay_flat(const TopoDS_Face &face);

  /**
   * @brief Mirror object across a plane
   * @param mirror_plane
   */
  void mirror(gp_Ax2 mirror_plane);

  /**
   * @brief mirrorXY
   */
  void mirrorXY() { mirror(gp_Ax2(center_point(), gp::DZ())); }

  /**
   * @brief mirrorXZ
   */
  void mirrorXZ() { mirror(gp_Ax2(center_point(), gp::DY())); }

  /**
   * @brief mirrorYZ
   */
  void mirrorYZ() { mirror(gp_Ax2(center_point(), gp::DX())); }

  /**
   * @brief Rotate object
   * @param axis Axis of rotation
   * @param angle Angle of rotation, in degrees
   */
  void rotate(const gp_Ax1 axis, const double angle);

  /**
   * @brief Rotate object on the X axis
   * @param angle Angle of rotation, in degrees
   */
  void rotateX(const double angle) {rotate(gp::OX(), angle);}

  /**
   * @brief rotateY
   * @param angle
   */
  void rotateY(const double angle) {rotate(gp::OY(), angle);}

  /**
   * @brief rotateZ
   * @param angle
   */
  void rotateZ(const double angle) {rotate(gp::OZ(), angle);}

  /**
   * @brief Translate object
   * @param x
   * @param y
   * @param z
   */
  void translate(const double x, const double y, const double z);

  /**
   * @brief scale
   * @param x
   * @param y
   * @param z
   */
  void scale(const double x, const double y, const double z);

  /**
   * @brief get_bound_box
   * @return
   */
  const Bnd_Box &get_bound_box() const { return this->bounding_box; }

  /**
   * @brief get_footprint
   * @return
   */
  const Bnd_Box2d &get_footprint() { return this->footprint; }

  /**
   * @brief width
   * @return
   */
  double width() const {
    return bounding_box.CornerMax().X() - bounding_box.CornerMin().X();
  }

  /**
   * @brief length
   * @return
   */
  double length() const {
    return bounding_box.CornerMax().Y() - bounding_box.CornerMin().Y();
  }

  /**
   * @brief height
   * @return
   */
  double height() const {
    return bounding_box.CornerMax().Z() - bounding_box.CornerMin().Z();
  }

  /**
   * @brief maxZ
   * @return
   */
  double maxZ() const { return bounding_box.CornerMax().Z(); }

  /**
   * @brief center_point
   * @return
   */
  const gp_Pnt center_point() const;

  /**
   * @brief get_volume
   * @return
   */
  double get_volume() const;

  /**
   * @brief get_shape
   * @return
   */
  const TopoDS_Shape get_shape() const { return shape;}

private:
  TopoDS_Shape shape;
  Bnd_Box bounding_box;
  Bnd_Box2d footprint;
  bool dirty;
};

/**
 * @brief operator <<
 * @param os
 * @param c
 * @return
 *
std::ostream &operator<<(std::ostream &os, const Object &c) {
  return os << "testing";
}
*/

} // namespace sse

