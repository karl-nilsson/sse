#include <sse/Slice.hpp>

/**
 * @brief Slice::Slice
 * @param _faces
 */
Slice::Slice(std::vector<TopoDS_Face> _faces) {
  spdlog::debug("Initializing slice");

  // for each face
  for (auto f : _faces) {
    faces.push_back(Face(f));
  }

  //    TopExp::MapShapes(f, TopAbs_WIRE, wires);
}

/**
 * @brief Slice::add_face add a face to the slice
 * @param f
 */
void Slice::add_face(TopoDS_Face face) {
  faces.push_back(Face(face));
}

/**
 * @brief Slice::get_faces return a list of faces in the slice
 */
auto Slice::get_faces() {
  return faces;
}


/**
 * @brief Face::Face
 * @param _face
 */
Face::Face(TopoDS_Face _face): face(_face){
  TopExp::MapShapes(face, TopAbs_WIRE, wires);
}
