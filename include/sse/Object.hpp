#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <BRepBndLib.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <BndLib.hxx>
#include <Bnd_Box.hxx>
#include <Bnd_Box2d.hxx>
#include <gp_Pln.hxx>
#include <gp.hxx>
#include <gp_Trsf.hxx>
#include <BRepBuilderAPI.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>

#include <AIS_Shape.hxx>
#include <GeomLProp_SLProps.hxx>
#include <error.h>

class Object{

public:
    Object(TopoDS_Shape s);
    void generate_bounds();
    void layFlat(const TopoDS_Face &face);
    void rotate(const gp_Ax1 axis, const double angle);
    void translate(const long x, const long y, const long z);
    void scale(const int x, const int y, const int z);
private:
    TopoDS_Shape shape;
    Bnd_Box bounding_box;
    Bnd_Box2d footprint;


};
