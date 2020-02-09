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

#include <sse/slicer.hpp>

namespace sse {

void init_log(unsigned int _loglevel = 3) {
  auto loglevel = spdlog::level::info;

  auto console = spdlog::stderr_color_mt("console");
  auto error_logger = spdlog::stderr_color_mt("stderr");
  spdlog::set_default_logger(console);

  // FIXME: probably a more elegant way to do this
  switch (_loglevel) {
  case 0:
    loglevel = spdlog::level::off;
    break;
  case 1:
    loglevel = spdlog::level::critical;
    break;
  case 2:
    loglevel = spdlog::level::err;
    break;
  case 3:
    loglevel = spdlog::level::warn;
    break;
  case 4:
    loglevel = spdlog::level::info;
    break;
  case 5:
    loglevel = spdlog::level::debug;
    break;
  default:
    loglevel = spdlog::level::info;
    break;
  }

  spdlog::set_level(loglevel);

  spdlog::info("Logger initialized");
}

void init_settings(fs::path configfile) {
  auto &s = sse::Settings::getInstance();
  spdlog::debug("Initializing settings");
  s.parse(configfile);
}

TopTools_ListOfShape make_tools(const double layer_height,
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

TopoDS_Face make_spiral_face(const double height, const double layer_height) {
  // make a unit cylinder, vertical axis, center @ (0,0), radius of 1
  Handle_Geom_CylindricalSurface cylinder =
      new Geom_CylindricalSurface(gp::XOY(), 1.0);
  // TODO: center helix axis should be the central print axis
  auto line = gp_Lin2d(gp::Origin2d(), gp_Dir2d(layer_height, 1.0));
  Handle_Geom2d_TrimmedCurve segment = GCE2d_MakeSegment(line, 0.0, M_PI * 2.0);
  // make the helixcal edge
  auto helixEdge =
      BRepBuilderAPI_MakeEdge(segment, cylinder, 0.0, 6.0 * M_PI).Edge();
  auto wire = BRepBuilderAPI_MakeWire(helixEdge);
  // make infinite line to sweep
  auto profile = NULL;
  // sweep line to create face
  // auto face = GeomFill_Pipe();
  // auto a = BRepOffsetAPI_MakePipe();
  auto face = TopoDS_Face();

  return face;
}

std::vector<Slice>
splitter(const std::vector<std::shared_ptr<Object>> &objects) {
  // find the highest z point of all objects
  double z = 0;
  auto obj = TopTools_ListOfShape();
  // FIXME: optimize, we copy the shape to another list
  for (auto o : objects) {
    z = std::max(z, o->get_bound_box().CornerMax().Z());
    obj.Append(o->get_shape());
  }

  Settings &s = Settings::getInstance();
  // FIXME more sane layer height fallback mechanism
  auto layer_height = toml::find_or<double>(s.config, "layer_height", 0.02);
  // spdlog::info("layer height: {}", layer_height);
  // create the slicing planes
  spdlog::info("creating slicing planes");
  auto tools = make_tools(layer_height, z);
  auto splitter = BRepAlgoAPI_Splitter{};
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
    auto report = splitter.GetReport();
    // TODO: dump error to spdlog
    spdlog::error("Error while splitting shape: ");
    splitter.DumpErrors(std::cerr);
    // throw error
  }

  auto result = std::vector<Slice>();
  // splitter.Shape() is a TopoDS compound, so iterate over it
  for (auto it = TopoDS_Iterator(splitter.Shape()); it.More(); it.Next()) {
    // skip over extraneous shapes created
    if (it.Value().ShapeType() == TopAbs_SOLID) {
      // add slice to list
      // FIXME: const_cast EVIL!
      // result.push_back(Slice(const_cast<TopoDS_Shape&>(it.Value())));
    }
  }
  // sort the slices by their height, ascending

  /*
  std::sort(result.begin(), result.end(), [](const auto &lhs, const auto &rhs) {
      return lhs.get_bound_box().CornerMin.Z() < rhs.get_bound_box().CornerMin.Z();
    });

  std::sort(result.begin(), result.end(), [](const auto a, const auto b){
      return true;
    });
    */

  return result;
}

void dump_shapes(const std::vector<TopoDS_Shape> shapes) {

  spdlog::debug("--------Shape Dump-------");
  for(auto s: shapes) {
      spdlog::debug(dump_recurse(s));
    }
  spdlog::debug("-------------------------");
}

void dump_shapes(const TopoDS_Shape &shape) {
  spdlog::debug("--------Shape Dump-------");
      spdlog::debug(dump_recurse(shape));
  spdlog::debug("-------------------------");
}

std::string dump_recurse(const TopoDS_Shape &shape) {
  // temporary string for return value
  std::string result;
  // prepend info with tree characters
  static int indent_level = 0;
  // construct prepend string based in indentation level
  std::string prepend;
  for(int i = 0; i < indent_level; ++i) {
      prepend += (i < indent_level -1 ? "|\t": "├─ ");
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

void section(const TopTools_ListOfShape &objects,
             const TopTools_ListOfShape &tools) {
  BOPAlgo_Section section;
  // get first object
  TopoDS_Shape object = objects.First();

  TopTools_ListOfShape result;

  for (auto face : tools) {
    // section object
    section.AddArgument(object);
    section.AddArgument(face);
    // section.BuildSection();
    result = section.Generated(object);
  }
}

void arrange_objects(std::vector<std::shared_ptr<Object>> objects) {
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

void make_build_volume() {
  // get build volume from settings
}

} // namespace sse
