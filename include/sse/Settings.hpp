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

  /**
   * @brief Parse toml file
   * @param _file File to parse
   */
  void parse(fs::path _file);

  /**
   * @brief Get a setting by name, with a designated fallback
   * @param setting Setting name
   * @param fallback Fallback value
   * @param return Setting if it exists, fallback otherwise
   *
   */
  template <typename T> T get_setting_fallback(std::string setting, T fallback) {
    return toml::find_or<T>(config, setting, fallback);
  }

  /**
   * @brief Get a setting
   * @param setting Setting name
   * @return return Setting
   */
  template <typename T> T get_setting(std::string setting) {
    return toml::find<T>(config, setting);
  }

  /**
   * @brief Dump settings to string
   * @return List of strings
   */
  std::string dump();

  /**
   * @brief Save settings to file
   */
  void save();

  // don't touch anything beneath here; required for singleton
  /**
   * @brief getInstance Get instance of settings
   * @return Settings instance
   */
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
