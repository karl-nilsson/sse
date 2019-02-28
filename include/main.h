#ifndef MAIN_H
#define MAIN_H

#include <TopTools.hxx>
#include <TopTools_ListOfShape.hxx>
#include <optional>

void slice(const TopTools_ListOfShape & objects);
TopTools_ListOfShape makeTools(const double layerHeight, const double objectHeight);
void debug_results(const TopoDS_Shape &result);
std::optional<TopoDS_Shape> splitter(const TopTools_ListOfShape &objects, const TopTools_ListOfShape &tools);

#endif // MAIN_H
