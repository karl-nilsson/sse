#include <sse/Settings.hpp>


Settings::Settings(fs::path file) {
  if(! fs::exists(file)) {
      std::cerr << "Error, config file " << file << " does not exist" << std::endl;
      return;
    }

  // auto config = cpptoml::parse_file(file);

}
