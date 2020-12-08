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
#include <sse/Importer.hpp>
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
  double layerheight, linewidth = 0.0;
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
      ("m,extrusion_multiplier", "Extrusion Multiplier", cxxopts::value<double>())
      ("speed", "Print speed, mm/s", cxxopts::value<double>())
      ("rapid", "Move speed, mm/s", cxxopts::value<double>())

      // placement group
      ("a,autoplace", "Automatically center/touch buildplate")

      // extrusion group
      ("l,layer_height", "Layer Height", cxxopts::value(layerheight))
      ("w,line_width", "Extrusion Width", cxxopts::value(linewidth))
      ("variable_layer", "Variable layer height", cxxopts::value<bool>())

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

  auto imp = sse::Importer{};
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
      TopoDS_Shape shape = imp.import(f);
      objects.push_back(make_unique<sse::Object>(shape));
    } catch (std::runtime_error &e) {
      cerr << e.what() << endl;
      continue;
    }

  }

  // auto arrange objects
  if (autoplace) {
    s.arrange_objects(objects);
  }

  std::vector<sse::Slice> slices;
  slices.reserve(objects.size() * 100);

  // cut the objects into slices
  for(const auto &o: objects) {
    auto result = s.slice_object(o.get());
    for(auto &slice: result) {
      s.generate_shells(slice);
      s.generate_infill(slice, 0, 0.6);
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
