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
#include <BRepGProp.hxx>
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
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
#include <Standard_Handle.hxx>
#include <StdFail_NotDone.hxx>

#include <math.h>
#include <memory>
#include <iostream>

#include <spdlog/spdlog.h>

namespace sse {

/**
 * @brief The Object class
 */
class Object {

public:

  /**
   * @brief Object constructor
   * @param shape Underlying shape
   * @param frame Filename
   */
  explicit Object(TopoDS_Shape &shape, const std::string &fname = "");

  /**
   * @brief Generate the bounding box
   * @param optimal Flag to generate optimal bounding box
   * @param gap Increase bounding box by $gap in each direction
   */
  void generate_bounds(bool optimal, double gap);

  /**
   * @brief Rotate and translate object so that one face is flat on the
   * buildplate
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
   * @brief Rotate object about center of object
   * @param axis Axis of rotation
   * @param angle Angle of rotation, in degrees
   */
  void rotate(const gp_Ax1 axis, const double angle);

  /**
   * @brief Rotate object about the X axis
   * @param angle Angle of rotation, in degrees
   */
  void rotateX(const double angle) { rotate(gp::OX(), angle); }

  /**
   * @brief Rotate object about the Y axis
   * @param angle Angle of rotation, in degrees
   */
  void rotateY(const double angle) { rotate(gp::OY(), angle); }

  /**
   * @brief Rotate object about the Z axis
   * @param angle Angle of rotation, in degrees
   */
  void rotateZ(const double angle) { rotate(gp::OZ(), angle); }

  /**
   * @brief Translate object
   * @param x X distance
   * @param y Y distance
   * @param z Z distance
   */
  void translate(const double x, const double y, const double z);

  /**
   * @brief Translate the object
   * @param v translation vector
   */
  void translate(const gp_Vec v);

  /**
   * @brief Translate the object
   * @param dest point of destination
   */
  void translate(const gp_Pnt dest);

  /**
   * @brief Scale the object
   * @param x
   * @param y
   * @param z
   */
  void scale(const double x, const double y, const double z);

  void transform(const gp_Trsf transform);

  /**
   * @brief Get the bounding box, aligned to the cartesian axes
   * @return bounding box
   */
  const Bnd_Box &get_bound_box() const { return this->bounding_box; }

  /**
   * @brief Get the bottom rectangle of the bounding box
   * @return
   */
  const Bnd_Box2d &get_footprint() { return this->footprint; }

  /**
   * @brief Get the X dimension of the bounding box
   * @return width
   */
  double width() const {
    return bounding_box.CornerMax().X() - bounding_box.CornerMin().X();
  }

  /**
   * @brief Get the Y dimension of the bounding box
   * @return length
   */
  double length() const {
    return bounding_box.CornerMax().Y() - bounding_box.CornerMin().Y();
  }

  /**
   * @brief Get the Z dimension of the bounding box
   * @return height
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
  inline TopoDS_Shape &get_shape() { return *shape; }

  friend std::ostream& operator<<(std::ostream& out, Object& o){
    // DumpJson only in occt-7.4
    // o.get_bound_box().DumpJson(out);
    return out;
  }

private:
  std::unique_ptr<TopoDS_Shape> shape;
  const std::string filename;
  Bnd_Box bounding_box;
  Bnd_Box2d footprint;
  std::string name;
};



} // namespace sse
