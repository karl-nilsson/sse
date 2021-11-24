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
// stl headers
#include <vector>
// external headers
#include <spdlog/spdlog.h>
#include "cavc/polylineoffsetislands.hpp"
// project headers
#include "sse/Object.hpp"
#include "sse/libsse_export.hpp"

namespace sse {

  // this struct simply cuts out the spatial index from the offsetloopset, because the former has a unique_ptr, thus can't be copied
  // TODO: figure out a better solution to this problem
  struct LIBSSE_EXPORT Shell {
    Shell(cavc::OffsetLoopSet<double> &loopset) {

      if(loopset.ccwLoops.size() != 1) {
        spdlog::error("Shell: Expected 1 outer polyline. received: {}", loopset.ccwLoops.size());
        throw std::invalid_argument("Shell: only 1 outer pline allowed");
      }

      outer = std::move(loopset.ccwLoops.front().polyline);

      islands.reserve(loopset.cwLoops.size());
      for(auto &loop: loopset.cwLoops) {
        islands.push_back(std::move(loop.polyline));
      }
    };

    Shell() = default;

    cavc::Polyline<double> outer;
    std::vector<cavc::Polyline<double>> islands;
  };

/**
 * @brief The Slice class
 */
class LIBSSE_EXPORT Slice {

public:

  /**
   * @brief Create a slice out of one planar face
   * @param face
   * @throw invalid_argument if face is null
   */
  explicit Slice(const Object *parent, TopoDS_Face face, double thickness);


  /**
   * @brief Generate shells for the slice
   * @param num_shells Number of shells (offsets) to generate
   * @param line_width Extrusion width (mm)
   * @param overlap Ratio of overlap between innermost shell and infill. 0 = no overlap, -1.0 = 1x line_width gap
   */
  void generate_shells(const int num_shells, const double line_width, const double overlap = 0.0);

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
   * @brief layer_thickness Get the thickness of the slice
   * @return slice thickness
   */
  [[nodiscard]] inline double layer_thickness() const noexcept {
    return this->thickness;
  }

  /**
   * @brief gcode Return gcode representation
   * @return GCode representation of moves
   */
  [[nodiscard]] std::string gcode(double extrusion_multiplier, double extrusion_width, double filament_diameter) const;


private:
  //! Parent object, from which this slice was created
  const Object *parent;
  //! face
  TopoDS_Face face;
  //! list of wires
  TopTools_ListOfShape wires;
  //! list of offsets
  std::vector<Shell> shells;
  //! innermost polyline(s), used for clipping infill
  Shell innermost_shell;
  //! infill
  std::vector<cavc::Polyline<double>> infill_polylines;
  //! z height
  double z;
  //! thickness, same as layer height
  double thickness;
};

} // namespace sse
