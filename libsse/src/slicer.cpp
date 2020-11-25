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
#include <GCE2d_MakeSegment.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <BOPAlgo_Section.hxx>
#include <TopExp_Explorer.hxx>
// external headers
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
// project headers
#include <sse/slicer.hpp>
#include <sse/Object.hpp>
#include <sse/Packer.hpp>

namespace sse {

Slicer::Slicer(const fs::path& configfile,
               const spdlog::level::level_enum loglevel)
    : settings(Settings::getInstance()) {
  // setup loggels
  auto console_logger = spdlog::stdout_color_mt("console");
  // auto error_logger = spdlog::stderr_color_mt("stderr");
  spdlog::set_default_logger(console_logger);
  // TODO: maybe unnecessary
  spdlog::flush_on(spdlog::level::info);
  // set log level
  spdlog::set_level(loglevel);
  spdlog::debug("Logger initialized");
  // parse settings
  spdlog::debug("Initializing settings");
  settings.parse(configfile);
}

TopTools_ListOfShape Slicer::make_tools(const double layer_height,
                                        const double object_height) {
  spdlog::info("Creating splitter tools");
  auto result = TopTools_ListOfShape{};
  // create an unbounded plane, parallel to the xy plane,
  // then convert it to a face
  for (int i = 0; i < object_height / layer_height + 1; ++i) {
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
  Handle_Geom2d_TrimmedCurve segment = GCE2d_MakeSegment(line, 0.0, M_PI * 2.0);
  // make the helixcal edge
  auto helixEdge =
      BRepBuilderAPI_MakeEdge(segment, cylinder, 0.0, 6.0 * M_PI).Edge();
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

std::vector<std::unique_ptr<Slice>>
Slicer::slice(const std::vector<std::shared_ptr<Object>> &objects) {
  // find the highest z point of all objects
  double z = 0;
  auto obj = TopTools_ListOfShape();
  // FIXME: optimize, we copy the shape to another list
  for (const auto& o : objects) {
    z = std::max(z, o->get_bound_box().CornerMax().Z());
    obj.Append(o->get_shape());
  }

  // FIXME more sane layer height fallback mechanism
  auto layer_height = settings.get_setting_fallback<double>("layer_height", 0.2);
  spdlog::info("Layer Height: {}", layer_height);
  // create the slicing planes
  spdlog::info("creating slicing planes");
  auto tools = make_tools(layer_height, z);
  auto splitter = BRepAlgoAPI_Splitter{};
  // TODO: progress indicator using BRepAlgoAPI_Splitter::SetProgressIndicator

  // set the arguments
  splitter.SetArguments(obj);
  splitter.SetTools(tools);
  // run in parallel
  splitter.SetRunParallel(true);
  // TODO: configurabe fuzzy value
  splitter.SetFuzzyValue(0.001);
  // run the algorithm
  splitter.Build();
  // check error status
  if (splitter.HasErrors()) {
    const auto& report = splitter.GetReport();
    report->Dump(std::cerr);
    // TODO: dump error to spdlog
    spdlog::error("Error while splitting shape: ");
    splitter.DumpErrors(std::cerr);
    // throw error
    throw std::runtime_error("Error splitting shapes");
  }

  auto slices = std::vector<std::unique_ptr<Slice>>();
  auto it = TopExp_Explorer();
  BRepBuilderAPI_Copy copy;
  // splitter.Shape() is a TopoDS compound, so iterate over it
  for (it.Init(splitter.Shape(), TopAbs_SOLID); it.More(); it.Next()) {
    // TODO: I don't like having to make a copy of the shape
    // iterator returns a const ref, so can't directly construct Slice
    // TODO: simplify
    copy.Perform(it.Current());
    auto a = copy.Shape();
    slices.push_back(std::make_unique<Slice>(a));
  }

  // sort the slices by height, ascending
  std::sort(slices.begin(), slices.end());
  // debug output
  spdlog::debug("number of slices: {}", slices.size());

  int num_shells = settings.get_setting_fallback<int>("shells", 3);
  auto extrusion_width =
      settings.get_setting_fallback<double>("extrusion_width", 0.4);
  spdlog::debug("generating shells");
  for (auto &s : slices) {
    s->generate_shells(num_shells, 1.0);
  }

  return slices;
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

void Slicer::arrange_objects(std::vector<std::shared_ptr<Object>> objects) {
  spdlog::debug("Creating Bin Packer");
  auto packer = Packer(objects);
  // pack the objects, get dimensions of resulting bin
  auto [width, length] = packer.pack();
  // check to see if the pack fit within the build plate
  // FIXME: get actual buildplate volume/dimensions
  double build_plate_x = 200, build_plate_y = 200;
  spdlog::debug("BinPack: comparing resulting bin to build plate size");
  if (width > build_plate_x || length > build_plate_y) {
    spdlog::debug("BinPack error: packed volume exceeds build plate");
    throw std::runtime_error("Bin Packing error: bin exceeds build plate");
  }
  // calculate the offset necessary for centering the pack on the build plate
  double offset_x = (build_plate_x - width) / 2;
  double offset_y = (build_plate_y - length) / 2;
  // translate the objects
  packer.arrange(offset_x, offset_y);
}

void Slicer::make_build_volume() {
  // get build volume from settings
}

} // namespace sse
