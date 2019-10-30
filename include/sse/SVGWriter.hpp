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
