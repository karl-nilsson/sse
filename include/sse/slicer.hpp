#pragma once

#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopAbs.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools.hxx>
#include <TopTools_ListOfShape.hxx>

#include <IFSelect.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <STEPControl_Reader.hxx>
#include <Standard.hxx>
#include <TDF.hxx>
#include <TDF_Attribute.hxx>

#include <GeomFill_Pipe.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom2d_Line.hxx>

#include <GCE2d_MakeSegment.hxx>

#include <gp.hxx>
#include <gp_Lin2d.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt2d.hxx>

#include <BOPAlgo_Section.hxx>
#include <BOPAlgo_Tools.hxx>
#include <BRepAlgo.hxx>
#include <BRepAlgoAPI_Splitter.hxx>
#include <BRepBuilderAPI.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepOffsetAPI_MakePipe.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <optional>

#include <sse/Importer.hpp>
#include <sse/Slice.hpp>
#include <sse/version.hpp>
#include <sse/Object.hpp>
#include <sse/Settings.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>


void init_log();

void init_settings(fs::path configfile);

TopTools_ListOfShape make_tools(const double layerHeight, const double objectHeight);

std::vector<TopoDS_Face> process_slice(TopoDS_Shape s, double z);

std::optional<TopoDS_Shape> splitter(const TopTools_ListOfShape &objects,
                                     const TopTools_ListOfShape &tools);

void slice(const TopTools_ListOfShape &objects);
TopTools_ListOfShape makeTools(const double layerHeight,
                               const double objectHeight);


void debug_results(const TopoDS_Shape &result);
std::optional<TopoDS_Shape> splitter(const TopTools_ListOfShape &objects,
                                     const TopTools_ListOfShape &tools);

void arrange_objects(std::vector<Object> objects);
