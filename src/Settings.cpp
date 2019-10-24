#include <sse/Settings.hpp>


Settings::Settings(fs::path file) {
  if(! fs::exists(file)) {
      spdlog::error("Error, config file " + file.string() + " does not exist");
      return;
    }

  spdlog::debug("Reading config file: " + file.string());
  auto config = toml::parse(file);

}
