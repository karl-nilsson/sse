#pragma once

#include <spdlog/spdlog.h>
#include <fmt/format.h>

#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>

#include <TopAbs.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <Standard_TypeMismatch.hxx>

#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRep_Tool.hxx>
#include <Standard_Real.hxx>

#include <Geom_BSplineCurve.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BoundedCurve.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Line.hxx>
#include <Geom_TrimmedCurve.hxx>

#include <sse/Settings.hpp>

// start off with a buffer size of 1MB
#define INITIAL_GCODE_SIZE 1048576

class GCodeWriter{

public:
    GCodeWriter();
    void add_comment(std::string comment);
    void add_rapid(int x, int y, int z);
    void add_segment(int x, int y, int z, int e, int f);
    void add_arc(int x, int y, int i, int j, int e, int f);
    void add_bezier(Geom_BezierCurve b);
    void add_bslpine(Geom_BSplineCurve b);
    void add_nurbs();
    void add_wire(TopoDS_Wire w);
    void retract();
    void purge();
    std::string get_data() {return this->data;}
private:
    std::string data;
    Settings &config;
};
