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

// std headers
#include <filesystem>
#include <cstdarg>
// external headers
#include <toml.hpp>

namespace fs = std::filesystem;

namespace sse {

/**
 * @brief The Settings class
 * Singleton
 */
class Settings {

public:

  /**
   * @brief Parse toml file
   * @param _file File to parse
   */
  void parse(fs::path _file);

  template <typename T> [[nodiscard]] T get_nested_setting(const char* setting, ...) {
    va_list args;
    va_start(args, setting);
    auto result = toml::find<T>(root, args);
    va_end(args);
    return result;
  }

  /**
   * @brief Get a setting by name, with a designated fallback
   * @param setting Setting name
   * @param fallback Fallback value
   * @return return Setting if it exists, fallback otherwise
   */
  template <typename T> [[nodiscard]] T get_setting_fallback(const std::string& setting, const T& fallback) {
    return toml::find_or(root, setting, fallback);
  }

  /**
   * @brief Get a setting
   * @param setting Setting name
   * @return return Setting
   */
  template <typename T> [[nodiscard]] T get_setting(const std::string& setting) {
    return toml::find<T>(root, setting);
  }

  /**
   * @brief Dump settings to string
   * @return List of strings
   */
  [[nodiscard]] std::string dump() const;

  /**
   * @brief Save settings to file
   */
  void save();

  // don't touch anything beneath here; required for singleton
  /**
   * @brief getInstance Get instance of settings
   * @return Settings instance
   */
  [[nodiscard]] static Settings &getInstance() {
    static Settings instance;
    return instance;
  }

  Settings(Settings const &) = delete;
  void operator=(Settings const &) = delete;

private:
  Settings() {}
  //! input file
  fs::path file;
  //! root node
  toml::value root;
};

} // namespace sse
