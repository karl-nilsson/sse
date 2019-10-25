#pragma once

#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <TCollection.hxx>
#include <TCollection_AsciiString.hxx>

#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>

#include <vector>
#include <map>
#include <unordered_map>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/fmt/ostr.h>


class Slice {

public:
  Slice();
  Slice(std::vector<TopoDS_Face> faces);
  void add_face(TopoDS_Face face);
  auto get_faces();
  auto get_wires() {return wires_map;}

private:
  std::map<TopoDS_Face,std::vector<TopoDS_Wire>> wires_map;
  TopTools_IndexedMapOfShape wires;
};

/**
 * @brief operator <<
 * @param os
 * @param c
 * @return
 *
std::ostream& operator<<(std::ostream& os, const Slice& c) {
  return os << "testing";
}
*/
