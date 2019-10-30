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
