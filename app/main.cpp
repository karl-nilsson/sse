// std headers
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits.h>
// external headers
#include <cxxopts.hpp>
#include <spdlog/spdlog.h>
// project headers
#include <sse/Object.hpp>
#include <sse/slicer.hpp>
#include <sse/version.hpp>


namespace fs = std::filesystem;
using namespace std;

/**
 * @brief main
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {

  // verbosity level
  int verbose = 0;
  int num_shells = 3;
  double layer_height, line_width = 0.0;
  double extrusion_multiplier = 1.0;
  double infill_density = 0.1;
  std::string infill_pattern;
  double nozzle_diameter = 0.4;
  double filament_diameter = 1.75;
  fs::path profile_filename;
  vector<string> files;
  bool autoplace = false;
  fs::path outfile;


  cxxopts::Options opts(argv[0], " - Slice CAD files for 3D printing");
  opts.positional_help("[optional args]").show_positional_help();

  // clang-format off
  opts.allow_unrecognised_options().add_options()
      // basic global settings
      ("h,help", "Help")
      ("v,verbose", "Verbosity")
      ("version", "Program Version")
      ("o,output", "Output File", cxxopts::value<string>())
      ("p,profile", "Settings profile", cxxopts::value<string>(), "FILE")
      // supports group
      ("supports", "Generate Supports", cxxopts::value<bool>())
      ("overhang", "Support", cxxopts::value<double>())
      // fan speed group
      ("fan_speed", "part cooling fan speed", cxxopts::value<double>())

      // speeds/feed group
      ("m,extrusion_multiplier", "Extrusion Multiplier. type: decimal, range: 0.0 - 1.0, default: 1.0", cxxopts::value<double>(extrusion_multiplier))
      ("speed", "Print speed, mm/s", cxxopts::value<double>())
      ("rapid", "Move speed, mm/s", cxxopts::value<double>())

      // printer group
      ("n,nozzle_diameter", "Nozzle Diameter (mm). type: decimal, default: 0.4")
      ("f,filament_diameter", "Filament Diameter (mm). type: decimal, default: 1.75")

      // placement group
      ("a,autoplace", "Automatically center/touch buildplate")

      // extrusion group
      ("l,layer_height", "Layer Height: type: decimal, default: 0.3", cxxopts::value(layer_height))
      ("s,shells", "Number of shells: type: integer, default: 3", cxxopts::value(num_shells))
      ("w,line_width", "Extrusion Width: type:decimal, default: 0.4", cxxopts::value(line_width))
      ("variable_layer", "Variable layer height: type: boolean, default: false", cxxopts::value<bool>())
      ("i,infill_density", "Infill density: type: decimal, range: 0.0 - 0.1, default: 0.1", cxxopts::value(infill_density))
      ("infill_pattern", "Infill pattern. type: string, values: rectilinear", cxxopts::value(infill_pattern))

      // positional, i.e. files to slice
      ("positional", "Positional arguments", cxxopts::value<vector<string>>());
  // clang-format on
  try {
    opts.parse_positional("positional");
    auto result = opts.parse(argc, argv);

    // print help message then quit
    if (result.count("help")) {
      cout << opts.help({"", "Group"}) << '\n';
      return 0;
    }

    // print version then quit
    if (result.count("version")) {
      fmt::print("sse version: {} git commit: {}\n", VERSION, GIT_TAG);
      return 0;
    }

    // automatically position models on the build plate
    if (result.count("autoplace")) {
      autoplace = true;
    }

    // load profile
    if (result.count("p")) {
      profile_filename = fs::path(result["profile"].as<string>());
      cout << "profile: " << profile_filename << '\n';
    }

    // positional args, i.e. files to slice
    if (result.count("positional")) {
      files = result["positional"].as<vector<string>>();
    } else {
      // no files to slice, error out
      cerr << "Error: no files provided\n";
    }

    if (result.count("o")) {
      outfile = fs::path(result["output"].as<string>());
      cout << "output file: " << outfile << '\n';
    }

  } catch (const cxxopts::OptionException &e) {
    // no files to slice, error and exit
    cerr << "ERROR PARSING OPTIONS: " << e.what() << '\n';
    exit(1);
  }

  // TODO: configurable log level
  // int loglevel = result.count("verbose");
  auto s = sse::Slicer(profile_filename);
  sse::setup_logger(spdlog::level::debug);

  auto objects = vector<unique_ptr<sse::Object>>();

  for (const auto &f : files) {
    cout << "Loading file: " << f << '\n';
    // check if file exists
    if (!fs::exists(f)) {
      cerr << "file " << f << " does not exist, skipping\n";
      continue;
    }
    try {
      // import the object, then add it to the list
      TopoDS_Shape shape = sse::import(f);
      objects.push_back(make_unique<sse::Object>(shape));
    } catch (std::runtime_error &e) {
      cerr << e.what() << endl;
      continue;
    }

  }

  // auto arrange objects
  // TODO: use config values
  if (autoplace) {
    sse::rearrange_objects(objects, 235.0, 235.0);
  }

  std::vector<sse::Slice> slices;
  slices.reserve(objects.size() * 100);

  // cut the objects into slices
  for(const auto &o: objects) {
    auto result = s.slice_object(o.get(), layer_height);
    for(auto &slice: result) {
      s.generate_shells(slice, line_width, num_shells);
      s.generate_infill(slice, infill_density, line_width);
    }
    slices.insert(slices.end(), result.begin(), result.end());
  }

  // turn the slices into gcode
  auto gcode = sse::collate_gcode(slices);

  ofstream outstream;
  outstream.open(outfile);

  if(!outstream) {
    cerr << "file: " << outfile << "could not be opened\n";
    return 1;
  }

  outstream << gcode << std::endl;
  outstream.close();

  return 0;
}
