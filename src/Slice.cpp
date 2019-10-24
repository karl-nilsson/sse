#include <sse/Slice.hpp>

/**
 * @brief Slice::Slice
 * @param _faces
 */
Slice::Slice(std::vector<TopoDS_Face> faces) {
  spdlog::debug("Initializing slice");

  wires_map = std::map<TopoDS_Face, std::vector<TopoDS_Wire>>();

  auto wire = TopoDS_Wire{};
  // for each face
  for (auto f : faces) {
    auto w = std::vector<TopoDS_Wire>{};
    // first wire is guaranteed to be the outer wire
    w.push_back(BRepTools::OuterWire(f));
    // explore wires
    for (BRepTools_WireExplorer exp(wire, f); exp.More(); exp.Next()) {
      // don't duplicate outer wire
      if (wire.IsSame(w[0])) {
        continue;
      }
      // add wire to vector
      w.push_back(wire);
    }
  }

  //    TopExp::MapShapes(f, TopAbs_WIRE, wires);
}

/**
 * @brief Slice::add_face add a face to the slice
 * @param f
 */
void Slice::add_face(TopoDS_Face face) {
  auto wire = TopoDS_Wire{};
  auto w = std::vector<TopoDS_Wire>{};
  w.push_back(BRepTools::OuterWire(face));

  for (BRepTools_WireExplorer exp(wire, face); exp.More(); exp.Next()) {
    if (wire.IsSame(w[0])) {
      continue;
    }
    w.push_back(wire);
  }
}

/**
 * @brief Slice::get_faces return a list of faces in the slice
 */
auto Slice::get_faces() {
  std::vector<TopoDS_Face> faces;
  faces.reserve(wires_map.size());
  for (const auto &f : wires_map) {
    faces.push_back(f.first);
  }

  return faces;
}
