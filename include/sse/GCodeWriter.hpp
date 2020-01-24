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
