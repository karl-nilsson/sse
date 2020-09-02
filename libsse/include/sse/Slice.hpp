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

/**
 * @file Slice.hpp
 * @brief
 *
 * @author Karl Nilsson
 *
 */

#pragma once

#include <TCollection.hxx>
#include <TCollection_AsciiString.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>

#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>

#include <GeomAbs_SurfaceType.hxx>

#include <map>
#include <unordered_map>
#include <vector>
#include <memory>

#include <spdlog/spdlog.h>

#include <sse/Object.hpp>

namespace sse {

/**
 * @brief The Slice class
 */
class Slice : public Object {

public:

  /**
   * @brief Create a slice, and generate child faces that are parallel to the XY plane and coincident with the z point specified
   * @param shape Underlying shape
   * @param z_pos Z height to generate child faces
   */
  explicit Slice(TopoDS_Shape &shape);

  /**
   * @brief Return all the bottom faces of the slice
   * @return list of faces
   */
  inline TopTools_HSequenceOfShape& get_faces() { return faces;}

  /**
   * @brief Generate shells for the slice
   * @param num
   * @param width
   */
  void generate_shells(int num, double width);

  // TODO: configurable infill pattern
  /**
   * @brief generate_infill
   * @param percent
   */
  void generate_infill(double percent, double angle, double line_width);

  /**
   * @brief operator < Comparator t
   * @param rhs Other slice to compare against
   * @return Whether the lowest point of this bounding box is above the lowest point in the other bounding box
   */
  bool operator <(const Slice& rhs) const;

private:
  //! list of faces
  TopTools_HSequenceOfShape faces;
  TopTools_ListOfShape wires;
};

} // namespace sse
