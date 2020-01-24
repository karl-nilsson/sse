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

#include <sse/Settings.hpp>

namespace sse {

void Settings::parse(fs::path _file) {
  file = _file;

  // check if file exists
  if (!fs::exists(file)) {
    spdlog::error("Error, config file " + file.string() + " does not exist");
    return;
  }

  spdlog::debug("Reading config file: " + file.string());

  // parse settings file
  config = toml::parse<toml::preserve_comments>(file);

  auto printer = toml::find(config, "printer");

  spdlog::debug("Printer: " + toml::find<std::string>(printer, "name"));

  auto build_plate = toml::find(printer, "build_plate");
  if (toml::find<bool>(build_plate, "is_circle")) {
    spdlog::debug("Build plate is circular");
  } else {
    spdlog::debug("Build plate is rectangular");
  }
}

/**
 * @brief
 * @return
 */
template <typename T> T Settings::get_setting_fallback(std::string setting, T _default) {
  return toml::find_or<T>(config, setting, _default);
}

template <typename T> T Settings::get_setting(std::string setting) {
  return toml::find<double>(config, setting);
}

/**
 * @brief Settings::dump
 * @return A string representation of all current settings
 */
std::string Settings::dump() {
  return "";
}

void Settings::save() {
  spdlog::info("Saving settings to " + file.string());
}

} // namespace sse
