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

/**
 * @brief makeTools
 * @param layerHeight
 * @param objectHeight
 * @return A list of tools (planar faces) to slice an object
 */
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

/**
 * @brief makeSpiralFace
 * @param height
 * @param radius
 * @return
 */
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

/**
 * @brief splitter Use the splitter algorithm to split a solid into slices
 * @param objects
 * @param tools
 * @return the list of shapes, or std::nullopt if failure
 */
std::optional<TopoDS_Shape> splitter(const std::vector<Object> objects) {
  // find the highest z point of all objects
  double z = 0;
  auto obj = TopTools_ListOfShape();
  for (auto o : objects) {
    z = std::max(z, o.get_bound_box().CornerMax().Z());
    obj.Append(o.get_shape());
  }


  Settings &s = Settings::getInstance();
  // FIXME
  auto layer_height = toml::find_or<double>(s.config, "layer_height", 0.02);

  auto tools = make_tools(layer_height, z);

  auto splitter = BRepAlgoAPI_Splitter{};
  // set the argument
  splitter.SetArguments(obj);
  splitter.SetTools(tools);
  // run in parallel
  splitter.SetRunParallel(true);
  splitter.SetFuzzyValue(0.0);
  // run the algorithm
  splitter.Build();
  // check error status
  if (splitter.HasErrors()) {
    std::cerr << "Error while splitting shape" << std::endl;
    splitter.DumpErrors(std::cerr);
    return std::nullopt;
  }

  // result of the operation result
  auto &result = splitter.Shape();
  debug_results(result);
  return result;
}

void make_slices(TopoDS_Shape slices) {
  // slices is a TopoDS compound, so we have to iterate over it
  for (auto it = TopoDS_Iterator(slices); it.More(); it.Next()) {
    const auto child = it.Value();
  }
}

/**
 * @brief process_slice
 * @param s
 * @return all faces parallel and coincident with the slicing plane
 */
std::vector<TopoDS_Face> process_slice(TopoDS_Shape s, double z) {
  auto faces = std::vector<TopoDS_Face>();

  // search the slice for faces parallel and coincident with slicing plane
  for (TopExp_Explorer exp(s, TopAbs_FACE); exp.More(); exp.Next()) {
    auto f = TopoDS::Face(exp.Value());
    Standard_Real umin, umax, vmin, vmax;
    BRepTools::UVBounds(f, umin, umax, vmin, vmax);
    // TODO: choose better U,V values
    auto props = GeomLProp_SLProps(BRep_Tool::Surface(f), (umin + umax) / 2,
                                   (vmin + vmax) / 2, 1, 1e-6);
    // if normal isn't the same as slicing plane, short-circuit
    // TODO: verify floating point equality, low epsilon
    if (gp::DZ().IsOpposite(props.Normal(), 0.01) &&
        fabs(props.Value().Z() - z) < pow(10, -6)) {
      // add face to the list of faces for the slice
      faces.push_back(f);
    }
  }

  return faces;
}

/**
 * @brief debug_results
 * @param result
 */
void debug_results(const TopoDS_Shape &result) {
  std::cout << TopAbs::ShapeTypeToString(result.ShapeType()) << std::endl;

  auto it = TopoDS_Iterator(result);
  for (; it.More(); it.Next()) {
    std::cout << TopAbs::ShapeTypeToString(it.Value().ShapeType()) << std::endl;
    it.Value().Location();
  }
}

/**
 * @brief section use the section algorithm to obtain a list of edges from an
 * intersection
 * @param objects
 * @param tools
 */
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

// center and arrange models on the buildplate
void arrange_objects(std::vector<Object> objects) {
  // TODO: use rectangle packing algo
  // move all objects to their new location
  for (auto o : objects) {
    o.translate(0, 0, 0);
  }
}

void make_build_volume() {
  // get build volume from settings
}

} // namespace sse
