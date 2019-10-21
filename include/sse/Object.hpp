#include <BRepBndLib.hxx>
#include <BRepBuilderAPI.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <BndLib.hxx>
#include <Bnd_Box.hxx>
#include <Bnd_Box2d.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <gp.hxx>
#include <gp_Pln.hxx>
#include <gp_Trsf.hxx>

#include <AIS_Shape.hxx>
#include <GeomLProp_SLProps.hxx>
#include <Standard_ConstructionError.hxx>
#include <StdFail_NotDone.hxx>

#include <math.h>

class Object {

public:
  Object(TopoDS_Shape s);
  void generate_bounds();
  void lay_flat(const TopoDS_Face &face);
  void mirror();
  void rotate(const gp_Ax1 axis, const double angle);
  void rotateX(const double angle);
  void rotateY(const double angle);
  void rotateZ(const double angle);
  void translate(const double x, const double y, const double z);
  void scale(const double x, const double y, const double z);
  const Bnd_Box &get_bound_box() {return this->bounding_box;}
  const Bnd_Box2d &get_footprint() {return this->footprint;}
  const double width() {return bounding_box.CornerMax().X() - bounding_box.CornerMin().X();}
  const double length() {return bounding_box.CornerMax().Y() - bounding_box.CornerMin().Y();}
  const double height() {return bounding_box.CornerMax().Z() - bounding_box.CornerMin().Z();}
  const double maxZ() {return bounding_box.CornerMax().Z();}
private:
  TopoDS_Shape shape;
  Bnd_Box bounding_box;
  Bnd_Box2d footprint;
};
