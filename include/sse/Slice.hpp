#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>

#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>

#include <vector>

class Slice {

public:
  Slice();
  Slice(std::vector<TopoDS_Face> _faces);
  Slice(TopoDS_Face face);
  void add_face(TopoDS_Face f);
  auto get_faces() { return faces; }
  auto get_wires();

private:
  std::vector<TopoDS_Face> faces;
  std::vector<TopoDS_Wire> wires;
};
