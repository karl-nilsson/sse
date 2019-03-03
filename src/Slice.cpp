#include <sse/Slice.hpp>

/**
 * @brief Slice::Slice
 * @param _faces
 */
Slice::Slice(std::vector<TopoDS_Face> _faces) {
    faces = _faces;

    // initialize list of wires
    wires = std::vector<TopoDS_Wire>();
}

/**
 * @brief Slice::add_face
 * @param f
 */
void Slice::add_face(TopoDS_Face f) {
    faces.push_back(f);
}

/**
 * @brief Slice::get_wires
 * first wire is guaranteed to be the outer wire
 */
auto Slice::get_wires() {
    auto w = std::vector<TopoDS_Wire>{};
    w.push_back(BRepTools::OuterWire(faces.at(0)));

    auto wire = TopoDS_Wire{};
    auto wire_explorer = BRepTools_WireExplorer{};
    for(auto f: faces) {
        for(wire_explorer.Init(wire, f); wire_explorer.More(); wire_explorer.Next());
    }
}

