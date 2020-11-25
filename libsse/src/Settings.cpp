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

// std headers
#include <utility>
// external headers
#include <spdlog/spdlog.h>
// project headers
#include <sse/Settings.hpp>

namespace sse {

void Settings::parse(fs::path _file) {
  file = std::move(_file);

  // TODO: more error checks: permissions
  // check if file exists
  if (!fs::exists(file)) {
    spdlog::error("Error, file does not exist: " + file.string());
    throw std::runtime_error("Error, file does not exist: " + file.string());
  }

  spdlog::debug("Reading config file: " + file.string());

  // parse settings file
  root = toml::parse<toml::preserve_comments>(file);

  auto printer = toml::find(root, "printer");

  spdlog::debug("Printer: " + toml::find<std::string>(printer, "name"));

  // load build plate
  auto build_plate = toml::find(printer, "build_plate");
  if (toml::find<bool>(build_plate, "is_circle")) {
    spdlog::debug("Build plate is circular");
  } else {
    spdlog::debug("Build plate is rectangular");
  }

}

std::string Settings::dump() const {
  return toml::format(root);
}

void Settings::save() {
  spdlog::info("Saving settings to " + file.string());

}

} // namespace sse
