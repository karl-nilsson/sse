#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <sse/Importer.hpp>
#include <sse/Object.hpp>
#include <sse/slicer.hpp>
#include <sse/version.hpp>

#include <cxxopts.hpp>

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
  string profile_filename;
  vector<string> files;
  bool autoplace = false;

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
      cout << "sse version " << VERSION << "\ngit sha: "
           << "VERSION_SHA" << '\n';
      return 0;
    }

    // adjust verbosity
    sse::init_log(result.count("verbose"));

    // automatically position models on the build plate
    if (result.count("autoplace")) {
      autoplace = true;
    }

    // load profile
    if (result.count("p")) {
      cout << "profile: " << result["profile"].as<string>() << '\n';
      auto profile = fs::path(result["profile"].as<string>());
      sse::init_settings(profile);
    }

    // positional args, i.e. files to slice
    if (result.count("positional")) {
      files = result["positional"].as<vector<string>>();
    } else {
      // no files to slice, error out
      cerr << "Error: no files provided\n";
    }

  } catch (const cxxopts::OptionException &e) {
    // no files to slice, error and exit
    cerr << "ERROR PARSING OPTIONS: " << e.what() << '\n';
    exit(1);
  }

  auto imp = sse::Importer{};
  auto objects = vector<shared_ptr<sse::Object>>();

  for (const auto &f : files) {
    cout << "Loading file: " << f << '\n';
    // check if file exists
    if (!fs::exists(f)) {
      cerr << "file " << f << " does not exist, skipping\n";
      continue;
    }
    // import the object, then add it to the list
    auto v = imp.importSTEP(f.c_str());

    if (v == nullopt) {
      cerr << "Error processing file " << f << '\n';
    } else {
      objects.push_back(make_shared<sse::Object>(v.value()));
    }
  }

  // auto arrange objects
  if (autoplace) {
    sse::arrange_objects(objects);
  }
  // slice the objects
  // auto result = sse::splitter(objects);
  // generate gcode


  return 0;
}
