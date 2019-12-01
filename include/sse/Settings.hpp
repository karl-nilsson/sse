#pragma once

#include <filesystem>
#include <iostream>
#include <map>
#include <vector>

#include <toml.hpp>

#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

namespace sse {

/**
 * @brief The Settings class
 * Singleton
 */
class Settings {

public:
  toml::value config;

  void parse(fs::path _file);

  template <typename T> T get_setting_fallback(std::string setting, T _default);
  template <typename T> T get_setting(std::string setting);

  std::string dump();
  void save();

  // don't touch anything beneath here; required for singleton
  static Settings &getInstance() {
    static Settings instance;
    return instance;
  }

  Settings(Settings const &) = delete;
  void operator=(Settings const &) = delete;

private:
  Settings() {}

  fs::path file;
};

} // namespace sse
