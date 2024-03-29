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

#pragma once

// std includes
#include <filesystem>
#include <memory>
#include <string>
#include <vector>
// OCCT includes
#include <TopTools_ListOfShape.hxx>
#include <TopoDS_Shape.hxx>
// external includes
#include <spdlog/spdlog.h>
// project includes
#include "sse/Slice.hpp"
#include "sse/Settings.hpp"
#include "sse/libsse_export.hpp"

#define SSE_MAXIMUM_NUM_OBJECTS 1000
#define SSE_FALLBACK_LAYER_HEIGHT 0.2
#define SSE_FALLBACK_NUM_SHELLS 3
#define SSE_FALLBACK_EXTRUSION_WIDTH 0.6


namespace fs = std::filesystem;

namespace sse {

/**
 * @brief collate_gcode Combine all gcode text into one string
 * @return
 */
[[nodiscard]] LIBSSE_EXPORT std::string collate_gcode(std::vector<Slice> &slices);


LIBSSE_EXPORT void setup_logger(spdlog::level::level_enum loglevel = spdlog::level::info);


/**
 * @brief Import solid model(s) from file
 * 
 * @param filename Source file
 * 
 * Valid extensions:
 *  - step
 *  - stepz
 *  - iges
 *  - brep
 *  - stl
 *  - obj
 * 
 * @return TopoDS_Shape 
 * @throws std::invalid_argument
 * - empty string
 * - file does not exist
 * - invalid file extension
 */
[[nodiscard]] LIBSSE_EXPORT TopoDS_Shape import(const std::string &filename);

/**
 * @brief rearrange Rearrange objects to fit the smallest footprint, centered on the bed
 *
 * @param objects List of objects to rearrange
 * @param bed_width X dimension of bed
 * @param bed_length Y dimension of bed
 *
 *
 * @throws std::invalid_argument
 * - objects contains a nullptr
 * - bed_width or bed_length <= 0
 * - objects.size() > SSE_MAXIMUM_NUM_OBJECTS
 * - any object footprint is larger than bed dimensions
 * - any object has zero footprint
 *
 * @throws std::runtime_error
 * - aggregate footprint of rearranged objects is larger than the bed
 * - cannot determine correct growth direction of bin
 */
LIBSSE_EXPORT void rearrange_objects(std::vector<std::unique_ptr<Object> > &objects, const double bed_width, const double bed_length);

class LIBSSE_EXPORT Slicer {
public:
  Slicer(const fs::path& configfile);


  /**
   * @brief init_settings
   * @param configfile
   */
  void init_settings(fs::path configfile);


  /**
   * @brief generate_infill
   * @param slice
   * @param infill_percent
   * @param line_width
   */
  void generate_infill(Slice &slice, const double infill_percent, const double line_width = SSE_FALLBACK_EXTRUSION_WIDTH);

  /**
   * @brief generate_shells Generate shells (offsets) for a slice's wires
   * use the
   * @param slice
   */
  void generate_shells(Slice &slice);

  /**
   * @brief generate_shells
   * @param slice
   * @param line_width extrusion width of each shell
   * @param count number of shells
   * @param overlap overlap between innermost shell and infill
   */
  void generate_shells(Slice &slice, const double line_width, const int count, const double overlap = 0.0);

  /**
   * @brief Slice a list of solids using the splitter algorithm
   * @param objects Objects to split
   * @return list of Slices
   */
  [[nodiscard]] std::vector<std::unique_ptr<Slice>>
  slice(const std::vector<std::unique_ptr<Object> > &objects);

  /**
   * @brief slice_object Slice an object using the common algorithm
   * @param object Object to slice
   * @return list of slices
   */
  [[nodiscard]] std::vector<Slice>
  slice_object(const Object * const object, double layer_height);


  /**
   * @brief Create a list of slicing planes
   * @param layerHeight Distance between planes
   * @param objectHeight Total height
   * @return A list of tools (planar faces) to slice an object
   */
  [[nodiscard]] TopTools_ListOfShape makeTools(const double layerHeight,
                                 const double objectHeight);

  /**
   * @brief makeSpiralFace
   * @param height
   * @param radius
   * @return
   */
  [[nodiscard]] TopoDS_Shape make_spiral_face(const double height, const double layer_height);

  /**
   * @brief Recursively dump shape info to log
   * @param result
   */
  void dump_shapes(const std::vector<TopoDS_Shape> &shapes);

  void dump_shapes(const TopoDS_Shape &shape);


  void make_build_volume();

  /**
   * @brief section use the section algorithm to obtain a list of edges from an
   * intersection
   * @param objects
   * @param tools
   */
  void section(const TopTools_ListOfShape &objects,
               const TopTools_ListOfShape &tools);

private:
  Settings &settings;

  [[nodiscard]] TopTools_ListOfShape make_tools(const double layer_height,
                                  const double object_height);

  [[nodiscard]] std::string dump_recurse(const TopoDS_Shape &shape);

  /**
   * @brief generate_gcode_header Generate header section of gcode file
   *
   * @param dump_settings Flag to include slicer settings in the header
   *
   * @return
   */
  [[nodiscard]] std::string generate_gcode_header(bool dump_settings = true);

  /**
   * @brief generate_gcode_footer
   * @return
   */
  [[nodiscard]] std::string generate_gcode_footer();
};

} // namespace sse
