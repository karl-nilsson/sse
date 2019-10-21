#include <algorithm>
#include <filesystem>
#include <iostream>
#include <vector>

#include <sse/Importer.hpp>
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

  init();

  // verbosity level
  int verbose = 0;
  double layerheight, linewidth = 0.0;
  string profile_filename;
  vector<string> files;


  cxxopts::Options opts(argv[0], " - Slice CAD files for 3D printing");
  opts.positional_help("[optional args]").show_positional_help();

  opts.allow_unrecognised_options().add_options()
      // basic global settings
      ("v,verbose", "Verbosity", cxxopts::value<int>())
      ("version", "Program Version")

      ("p,profile", "Settings profile", cxxopts::value<string>(), "FILE")
      // supports group
      ("s,supports", "Generate Supports", cxxopts::value<bool>())
      ("o,overhang", "Support", cxxopts::value<bool>())
      // fan speed group

      // speeds/feed group
      ("m,extrusion_multiplier", "Extrusion Multiplien",cxxopts::value<double>())
      ("speed", "Print speed, mm/s", cxxopts::value<double>())
      ("rapid", "Move speed, mm/s", cxxopts::value<double>())

      // placement group
      ("a,autoplace", "Automatically center/touch buildplate")

      // extrusion group
      ("l,layer_height", "Layer Height", cxxopts::value(layerheight))
      ("w,line_width", "Extrusion Width", cxxopts::value(linewidth))

      // positional, i.e. files to slice
      ("positional", "Positional arguments", cxxopts::value<vector<string>>());

  try {
    opts.parse_positional("positional");
    auto result = opts.parse(argc, argv);

    // print help message then quit
    if (result.count("help")) {
      cout << opts.help({"", "Group"}) << endl;
      return 0;
    }

    // adjust verbosity
    if (result.count("verbose")) {
      verbose = result["verbose"].as<int>();
      cout << "verbosity level=" << verbose << endl;
    }

    if (result.count("autoplace")) {
      cout << "autoplace" << endl;
    }

    // load profile
    if (result.count("p")) {
      auto profile = fs::path(result["profile"].as<string>());

      if (fs::exists(profile)) {
        // load json profile
      } else {
        cerr << "profile file " << profile << " does not exist, skipping"
             << endl;
      }
    }

    // positional args, i.e. files to slice
    if (result.count("positional")) {
      files = result["positional"].as<vector<string>>();
    } else {
      // no files to slice, error out
      cerr << "Error: no files provided" << endl;
    }

  } catch (const cxxopts::OptionException &e) {
    // no files to slice, error and exit
    cerr << "ERROR PARSING OPTIONS: " << e.what() << std::endl;
    exit(1);
  }

  auto imp = Importer{};

  auto objects = vector<TopoDS_Shape>();

  for (const auto &f : files) {
    cout << "Loading file: " << f << endl;
    // check if file exists
    if (!fs::exists(f)) {
      cerr << "file " << f << " does not exist, skipping" << endl;
      continue;
    }
    // import the object, then add it to the list
    auto v = imp.importSTEP(f.c_str());
    if (v == std::nullopt) {
      cerr << "Error processing file " << f << endl;
    } else {
      objects.push_back(v.value());
    }
  }


  return 0;
}
