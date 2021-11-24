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

// std headers
#include <math.h>
#include <algorithm>
#include <iostream>
#include <utility>
#include <exception>
#include <stdexcept>
#include <chrono>
#include <set>
#include <vector>
// OCCT headers
#include <gp_Pln.hxx>
#include <gp_Lin2d.hxx>
#include <Standard_Handle.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <BRepOffsetAPI_MakePipeShell.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <BRepAlgoAPI_Splitter.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <gce_ErrorType.hxx>
#include <GCE2d_MakeSegment.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS.hxx>
#include <BOPAlgo_Section.hxx>
#include <TopExp_Explorer.hxx>
// external headers
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <cavc/polyline.hpp>
#include <spdlog/cfg/env.h>
// project headers
#include <sse/slicer.hpp>
#include <sse/Object.hpp>
#include <sse/version.hpp>

using namespace fmt::literals;

namespace sse {

Slicer::Slicer(const fs::path& configfile) : settings(Settings::getInstance()) {

  if(! configfile.empty()) {
    spdlog::debug("Initializing settings");
    settings.parse(configfile);
  }

}

void setup_logger(spdlog::level::level_enum loglevel) {
  if(spdlog::get("console")) {
    spdlog::warn("Logger already exists");
    return;
  }

  // setup loggers
  auto console_logger = spdlog::stdout_color_mt("console");
  spdlog::set_default_logger(console_logger);
  // set log level
  spdlog::set_level(loglevel);
  // environment variable overrides provided log level
  spdlog::cfg::load_env_levels();
  spdlog::debug("Logger initialized, level: {}", console_logger->level());

}

constexpr const char* gce_ErrorString(const gce_ErrorType &e) {
  switch(e) {
    case gce_Done: return "completed";
    case gce_ConfusedPoints: return "coincident points";
    case gce_NegativeRadius: return "negative radius";
    case gce_ColinearPoints: return "colinear points";
    case gce_IntersectionError: return "invalid intersection";
    case gce_NullAxis:  return "undefined axis";
    case gce_NullAngle: return "undefined angle";
    case gce_NullRadius: return "undefined radius";
    case gce_InvertAxis: return "inverted axis";
    case gce_BadAngle: return "invalid angle";
    case gce_InvertRadius: return "inverted radius";
    case gce_NullVector: return "undefined vector";
    case gce_NullFocusLength: return "undefined focal distance";
    case gce_BadEquation: return "invalid equation";
    default: return "unknown error type";
  }
}

TopTools_ListOfShape Slicer::make_tools(const double layer_height,
                                        const double object_height) {
  spdlog::info("Creating splitter tools");
  auto result = TopTools_ListOfShape{};
  // create an unbounded plane, parallel to the xy plane,
  // then convert it to a face
  // n.b. position the plane in the top of the layer (e.g. 0.4x for 0.4mm layers)
  for (int i = 1; i < object_height / layer_height + 1; ++i) {
    result.Append(BRepBuilderAPI_MakeFace(
        gp_Pln(gp_Pnt(0, 0, i * layer_height), gp::DZ())));
  }
  return result;
}

TopoDS_Shape make_spiral_face(const double height, const double layer_height) {
  // TODO: use settings class
  // find the center of the bed
  double build_plate_x = 200, build_plate_y = 200;
  double center_x = build_plate_x / 2, center_y = build_plate_y / 2;
  auto center_point = gp_Pnt(center_x, center_y, 0);

  // create unit cylinder, at the center of the buildplate, vertical axis, radius=1
  Handle_Geom_CylindricalSurface cylinder =
      new Geom_CylindricalSurface(gp_Ax2(center_point, gp::DZ()), 1.0);
  // TODO: center helix axis should be the central print axis
  auto line = gp_Lin2d(gp::Origin2d(), gp_Dir2d(layer_height, 1.0));
  auto segment = GCE2d_MakeSegment(line, 0.0, M_PI * 2.0);
  if(segment.Status() != gce_Done) {
    spdlog::error("Trimming segment failed: {}", gce_ErrorString(segment.Status()));
    throw std::runtime_error(gce_ErrorString(segment.Status()));
  }
  // make the helical edge
  auto helixEdge =
      BRepBuilderAPI_MakeEdge(segment.Value(), cylinder, 0.0, 6.0 * M_PI).Edge();
  auto path = BRepBuilderAPI_MakeWire(helixEdge).Wire();
  // make line to sweep
  auto profile = BRepBuilderAPI_MakeEdge(gp_Ax1(center_point, gp_Dir(1,0,0))).Edge();
  // sweep line to create face
  auto result = BRepOffsetAPI_MakePipeShell(path);
  // set to freenet
  result.SetMode(Standard_True);
  result.Add(profile);
  result.Build();
  // list of results
  //result.Generated();

  return result.FirstShape();
}

cavc::Polyline<double> generate_infill_pattern(const double infill_density, const double line_width, const double bed_width, const double bed_length) {
  cavc::Polyline<double> infill_pattern;
  infill_pattern.isClosed() = false;
  const auto xmin = 0.0, ymin = 0.0;
  // for rectilinear infill, infill% = num lines * line width / face width
  int num_lines = infill_density * bed_width / line_width;
  // calculate offset between lines;
  double offset = bed_width / num_lines;

  // vertical zig-zag pattern
  double x = xmin, y = bed_length;
  int i = 0;
  while(x < bed_width) {
    infill_pattern.addVertex(x, y, 0);
    x += offset;
    infill_pattern.addVertex(x, y, 0);
    y = (i % 2) ? ymin : bed_length;
    ++i;
  }

  // end with a line segment back to the origin
  infill_pattern.addVertex(x, ymin - 10, 0);
  infill_pattern.addVertex(0, 0, 0);

  return infill_pattern;
}

void Slicer::generate_infill(Slice &slice, const double infill_density, const double line_width) {
  // TODO: replace these values with bounds of printer bed
  auto bed_width = settings.get_setting_fallback<double>("printer.build_plate.Width", 500);
  auto bed_length = settings.get_setting_fallback<double>("printer.build_plate.length", 500);

  auto infill_pattern = generate_infill_pattern(infill_density, line_width, bed_width, bed_length);

  slice.generate_infill(infill_pattern);

}


void Slicer::generate_shells(Slice &slice) {

  // TODO: better shells/extrusion width selection
  int num_shells = settings.get_setting_fallback<int>("shells", FALLBACK_NUM_SHELLS);
  auto line_width =
      settings.get_setting_fallback<double>("extrusion_width", FALLBACK_EXTRUSION_WIDTH);

  generate_shells(slice, line_width, num_shells);
}

void Slicer::generate_shells(Slice& slice, const double line_width, const int count, const double overlap) {
  if(line_width <= 0) {
    throw std::invalid_argument("Line width, must be > 0");
  }

  if(count <= 0) {
    throw std::invalid_argument("Shell count must be > 0");
  }

  spdlog::debug("generating shells");
  slice.generate_shells(count, line_width, overlap);

}

std::vector<Slice>
Slicer::slice_object(const Object * const object, double layer_height) {
  // find the z max
  double z = object->get_bound_box().CornerMax().Z();

  TopTools_ListOfShape args;
  args.Append(object->get_shape());

  // FIXME more sane layer height fallback mechanism
  // auto layer_height = settings.get_setting_fallback<double>("layer_height", FALLBACK_LAYER_HEIGHT);
  spdlog::info("Layer Height: {}", layer_height);
  auto tools = make_tools(layer_height, z);
  BRepAlgoAPI_Common common;
  // TODO: progress indicator using BRepAlgoAPI_Common::SetProgressIndicator

  // set the arguments
  common.SetArguments(args);
  common.SetTools(tools);
  // run in parallel
  common.SetRunParallel(true);
  // TODO: configurabe fuzzy value
  common.SetFuzzyValue(0.001);
  // run the algorithm
  common.Build();
  // check error status
  if (common.HasErrors()) {
    const auto& report = common.GetReport();
    report->Dump(std::cerr);
    // TODO: dump error to spdlog
    spdlog::error("Error while splitting shape: ");
    common.DumpErrors(std::cerr);
    // throw error
    throw std::runtime_error("Error splitting shapes");
  }

  std::vector<Slice> slices;
  slices.reserve((size_t)(z / layer_height));
  auto it = TopExp_Explorer();
  // disregard non-face (i.e. point, wire, edge) entities
  for (it.Init(common.Shape(), TopAbs_FACE); it.More(); it.Next()) {
    try {
      slices.emplace_back(object, TopoDS::Face(it.Current()), layer_height);
    }  catch (const Standard_TypeMismatch &e) {
      e.Print(std::cerr);
      spdlog::error("Error creating a TopoAbs_Face out of slice object");
    }
  }

  spdlog::debug("number of slices: {}", slices.size());

  return slices;
}

std::string generate_gcode_header(bool dump_settings) {
  std::string result;
  result.reserve(100);

  std::string short_sha = GIT_SHA1;
  short_sha = short_sha.substr(0,8);
  auto now =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  result += fmt::format(";Sliced by StepSlicerEngine v{:s} r{:s}, on {}\n", VERSION, short_sha, std::ctime(&now));

  result += ";FLAVOR:Marlin\n"
            ";Layer height:{layer_height}\n"
            "M104 S{hotend_temp:f}; Set hotend temp\n"
            "M190 S{bed_temp:f}; Set bed temp and wait\n"
            "M105\n"
            "M109; wait for hotend temp\n"
            "M105\n"
            "M82; Absolute extrusion\n"
            "G92 E0; Reset extruder position\n"
            "G28; Home all axes\n"
            "M106 S{fan_speed:d}; set fan speed\n"
            ";LAYER_COUNT:{layer_count}\n";


  return result;

}

std::string generate_gcode_footer() {

  std::string result;

  result += "G1 X0 Y235 ;Present print\n"
            "M104 S0 ;Turn off hotend\n"
            "M140 S0 ;Turn off bed\n"
            "M106 S0 ;Turn off fan\n"
            "M84 X Y E ;Disable all steppers except Z\n";


  return result;

}

std::string collate_gcode(std::vector<Slice> &slices) {
  std::string result;

  if(slices.empty()) {
    spdlog::warn("Slicer: no slices provided");
    return result;
  }

  // reserve 10MiB
  result.reserve((size_t)(10 << 20));
  // kill when gcode file exceeds 1GiB
  // TODO: consider preprocessor/env var
  constexpr size_t max_string_size = 1 << 30;



  // TODO: implement better ordering
  // currently, this simply sorts the slices by z-position, ascending
  spdlog::debug("sorting slices");
  std::sort(slices.begin(), slices.end(),
    [](const Slice& lhs, const Slice& rhs){
      return lhs.z_position() < rhs.z_position();
  });

  std::set<double> layers_set;

  for(const auto& slice: slices) {
    layers_set.insert(slice.z_position());
  }

  auto layer_count = layers_set.size();
  double layer_height = slices.front().layer_thickness();
  double hotend_temp = 225;
  double bed_temp = 65;
  int fan_speed = 255;
  double extrusion_multiplier = 1.0;
  double filament_diameter = 1.75;
  double extrusion_width = 0.6;


  spdlog::trace("adding gcode header");
  result += fmt::format(generate_gcode_header(true),
                        "layer_height"_a = layer_height,
                        "layer_count"_a = layer_count,
                        "hotend_temp"_a = hotend_temp,
                        "bed_temp"_a = bed_temp,
                        "fan_speed"_a = fan_speed);

  int current_layer_number = -1;
  double current_layer = -1;


  for(const auto& slice: slices) {
    auto slice_gcode = slice.gcode(filament_diameter, extrusion_width, extrusion_multiplier);

    if(result.size() + slice_gcode.size() > max_string_size) {
      spdlog::error("GCode string size {:d}MiB exceeded maximum size: {:d}MiB",
                    (result.size() + slice_gcode.size()) >> 20 ,
                    max_string_size >> 20
                    );
      throw std::runtime_error("GCode getting too big, bailing");
    }

    // add comment and move command on layer change
    if(slice.z_position() > current_layer) {
      current_layer = slice.z_position();
      current_layer_number++;
      result += fmt::format(";LAYER: {:d}\n", current_layer_number);
      // TODO: layer hop, configurable feedrate
      result += (fmt::format("G0 Z{:.6f} F5000\n", current_layer));
    }

    result.append(slice_gcode);
  }

  result += generate_gcode_footer();


  return result;
}

void Slicer::dump_shapes(const std::vector<TopoDS_Shape> &shapes) {
  spdlog::debug("--------Shape Dump-------");
  for (const auto &s : shapes) {
    spdlog::debug(dump_recurse(s));
  }
  spdlog::debug("-------------------------");
}

void Slicer::dump_shapes(const TopoDS_Shape &shape) {
  spdlog::debug("--------Shape Dump-------");
  spdlog::debug(dump_recurse(shape));
  spdlog::debug("-------------------------");
}

std::string Slicer::dump_recurse(const TopoDS_Shape &shape) {
  // temporary string for return value
  std::string result;
  // prepend info with tree characters
  static int indent_level = 0;
  // construct prepend string based in indentation level
  std::string prepend;
  for (int i = 0; i < indent_level; ++i) {
    prepend += (i < indent_level - 1 ? "|\t" : "├─ ");
  }

  // TODO: provide more information
  result += prepend + TopAbs::ShapeTypeToString(shape.ShapeType()) + "\n";

  // if shape has sub-shapes, recurse
  if (shape.ShapeType() == TopAbs_COMPOUND) {
    // increase indentation level for subtree
    indent_level++;
    for (auto it = TopoDS_Iterator(shape); it.More(); it.Next()) {
      // special character for final entry in list
      result += dump_recurse(it.Value());
    }
    // when subtree is exhausted, decrement indentation level
    indent_level--;
  }

  return result;
}

void Slicer::section(const TopTools_ListOfShape &objects,
                     const TopTools_ListOfShape &tools) {
  BOPAlgo_Section section;
  // get first object
  const TopoDS_Shape& object = objects.First();

  TopTools_ListOfShape result;

  for (const auto& face : tools) {
    // section object
    section.AddArgument(object);
    section.AddArgument(face);
    // section.BuildSection();
    result = section.Generated(object);
  }
}

void rearrange_objects(std::vector<std::unique_ptr<Object>> &objects) {
  auto &settings = Settings::getInstance();
  // TODO: finalize settings schema
  auto bed_width = settings.get_setting_fallback("printer.bed.width", 300);
  auto bed_length = settings.get_setting_fallback("printer.bed.length", 300);

  rearrange_objects(objects, bed_width, bed_length);

}

void Slicer::make_build_volume() {
  // get build volume from settings
}

} // namespace sse
