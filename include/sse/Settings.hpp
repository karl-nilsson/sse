#pragma once

#include <filesystem>
#include <iostream>
#include <map>
#include <vector>

// #include <toml.hpp>

#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

namespace sse {

/**
 * @brief The Settings class
 * Singleton
 */
class Settings {

public:
  void read_file(fs::path file);

  template <typename T> T get_setting(std::string setting);

  // don't touch anything beneath here
  static Settings &getInstance() {
    static Settings instance;
    return instance;
  }

  Settings(Settings const &) = delete;
  void operator=(Settings const &) = delete;

private:
  Settings() {}

  //    toml_config config;
};

} // namespace sse
