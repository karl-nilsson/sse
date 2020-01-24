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

#include <simple_svg.hpp>

#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

namespace sse {

class SVGWriter {
public:
  SVGWriter(fs::path _file);
  void add_slice();
  void add_line(double x1, double y1, double x2, double y2);
  void add_path(std::vector<svg::Point> points, bool bezier);
  void save();
  const svg::Stroke outline = svg::Stroke(0.5, svg::Color::Blue);
  const svg::Stroke infill = svg::Stroke(0.5, svg::Color::Red);
  const svg::Stroke brim = svg::Stroke(0.5, svg::Color::Green);
  const svg::Stroke support = svg::Stroke(0.5, svg::Color::Aqua);

private:
  svg::Document doc;
};
} // namespace sse
