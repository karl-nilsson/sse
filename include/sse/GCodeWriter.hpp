#pragma once

#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <chrono>

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
#include <GeomAdaptor.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomAdaptor_HCurve.hxx>

#include <sse/Settings.hpp>

// start off with a buffer size of 1MB
#define INITIAL_GCODE_SIZE 1048576

class GCodeWriter{

public:
    GCodeWriter();
    void create_header();
    void add_comment(std::string comment);
    void add_rapid(double x, double y, double z);
    void add_line(GeomAdaptor_Curve c);
    void add_arc(GeomAdaptor_Curve c);
    void add_bezier(Geom_BezierCurve b);
    void add_bslpine(Geom_BSplineCurve b);
    void add_nurbs();
    std::string add_segment(GeomAdaptor_Curve c);
    void add_wire(TopoDS_Wire w);
    void retract(double distance);
    void purge();
    std::string get_data() {return this->data;}
private:
    std::map<double,std::vector<std::string>> data_map;
    std::string data;
    sse::Settings &config;
    void move_pre(GeomAdaptor_Curve c);
};
