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

class Object {

public:
  Object(const Object& object);
  Object(TopoDS_Shape s);
  void generate_bounds();
  void lay_flat(const TopoDS_Face &face);
  void mirror(gp_Ax2 mirror_plane);
  void mirrorXY() { mirror(gp_Ax2(center_point(), gp::DZ())); }
  void mirrorXZ() { mirror(gp_Ax2(center_point(), gp::DY())); }
  void mirrorYZ() { mirror(gp_Ax2(center_point(), gp::DX())); }
  void rotate(const gp_Ax1 axis, const double angle);
  void rotateX(const double angle);
  void rotateY(const double angle);
  void rotateZ(const double angle);
  void translate(const double x, const double y, const double z);
  void scale(const double x, const double y, const double z);
  const Bnd_Box &get_bound_box() { return this->bounding_box; }
  const Bnd_Box2d &get_footprint() { return this->footprint; }
  const double width() {
    return bounding_box.CornerMax().X() - bounding_box.CornerMin().X();
  }
  const double length() {
    return bounding_box.CornerMax().Y() - bounding_box.CornerMin().Y();
  }
  const double height() {
    return bounding_box.CornerMax().Z() - bounding_box.CornerMin().Z();
  }
  const double maxZ() { return bounding_box.CornerMax().Z(); }
  const gp_Pnt center_point();

  const double get_volume();

  const TopoDS_Shape get_shape() { return shape;}

private:
  TopoDS_Shape shape;
  Bnd_Box bounding_box;
  Bnd_Box2d footprint;
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

