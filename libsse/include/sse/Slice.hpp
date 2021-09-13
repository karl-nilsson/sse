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
// OCCT headers
#include <TopTools_HSequenceOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
// project headers
#include <sse/Object.hpp>
// stl headers
#include <vector>

#include <spdlog/spdlog.h>
// external headers
#include "cavc/polylineoffsetislands.hpp"

namespace sse {

  // this struct simply cuts out the spatial index from the offsetloopset, because the former has a unique_ptr, thus can't be copied
  // TODO: figure out a better solution to this problem
  struct Shell {
    Shell(cavc::OffsetLoopSet<double> &loopset) {
      if(loopset.ccwLoops.size() != 1) {
        spdlog::error("Shell: Expected 1 outer polyline. received: {}", loopset.ccwLoops.size());
        throw std::invalid_argument("Shell: only 1 outer pline allowed");
      }

      outer = loopset.ccwLoops.front().polyline;

      islands.reserve(loopset.cwLoops.size());
      for(auto &loop: loopset.cwLoops) {
        islands.push_back(loop.polyline);
      }
    };

    cavc::Polyline<double> outer;
    std::vector<cavc::Polyline<double>> islands;
  };

/**
 * @brief The Slice class
 */
class Slice {

public:

  /**
   * @brief Create a slice out of one planar face
   * @param face
   * @throw invalid_argument if face is null
   */
  explicit Slice(const Object *parent, TopoDS_Face face, double thickness);


  /**
   * @brief Generate shells for the slice
   * @param offsets vector of offsets, positive = outward facing, negative = inward
   */
  void generate_shells(std::vector<double>& offsets_list);

  /**
   * @brief Generate infill for the slice
   *
   * Use polyline intersect and subtract to create infill within the face
   *
   * @param infill_pattern Polyline of the infill pattern to use
   */
  void generate_infill(cavc::Polyline<double> infill_pattern);

  /**
   * @brief z_position Return Z position
   * @return Z position
   */
  [[nodiscard]] inline double z_position() const noexcept {
    return z;
  }

  /**
   * @brief gcode
   * @return GCode representation of moves
   */
  [[nodiscard]] std::string gcode() const;

private:
  //! Parent object, from which this slice was created
  const Object *parent;
  //! face
  TopoDS_Face face;
  //! list of wires
  TopTools_ListOfShape wires;
  //! list of offsets
  std::vector<Shell> shells;
  //! innermost polyline(s)
  Shell *innermost_shell = nullptr;
  //! infill
  std::vector<cavc::Polyline<double>> infill_polylines;
  //! z height
  double z;
  //! thickness, same as layer height
  double thickness;
};

} // namespace sse
