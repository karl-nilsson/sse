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

#include <sse/SVGWriter.hpp>

namespace sse {

SVGWriter::SVGWriter(fs::path _file)
    : doc(_file,
          svg::Layout(svg::Dimensions(1000, 1000), svg::Layout::BottomLeft)) {}

void SVGWriter::add_line(double x1, double y1, double x2, double y2) {
  auto l = svg::Line(svg::Point(x1, y1), svg::Point(x2, y2), outline);

  doc << l;
}

void SVGWriter::add_path(std::vector<svg::Point> points, bool bezier) {

  auto path = svg::Path(outline);

  for (auto p : points) {
    path << p;
  }

  doc << path;
}

void SVGWriter::save() { doc.save(); }

} // namespace sse
