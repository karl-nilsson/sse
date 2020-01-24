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
