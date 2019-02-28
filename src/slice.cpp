#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>

#include <IFSelect.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <STEPControl_Reader.hxx>
#include <Standard.hxx>
#include <TDF.hxx>
#include <TDF_Attribute.hxx>
#include <TopAbs.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools.hxx>

#include <Geom_CylindricalSurface.hxx>
#include <GeomFill_Pipe.hxx>

#include <gp_Pln.hxx>
#include <gp_Lin2d.hxx>
#include <gp_Pnt2d.hxx>
#include <Geom2d_Line.hxx>
#include <GCE2d_MakeSegment.hxx>


#include <BOPAlgo_Section.hxx>
#include <BOPAlgo_Tools.hxx>
#include <BRepAlgo.hxx>
#include <BRepAlgoAPI_Splitter.hxx>

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>

#include <BRepOffsetAPI_MakePipe.hxx>

#include <BRepTools_WireExplorer.hxx>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "importer.h"
#include "slice.h"
#include "version.h"

namespace fs = std::filesystem;

/**
 * @brief main
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {
  cout << "StepSlicerEngine V0.01A" << endl;

  if (argc < 2) {
    cerr << "Usage: " << argv[0] << "file1.step" << endl;
    return 1;
  }

  auto imp = Importer{};

  //Handle(TopTools_HSequenceOfShape) shapes = new TopTools_HSequenceOfShape();
  //shapes->Clear();
  auto shapes = TopTools_HSequenceOfShape{};
  shapes.Clear();

  for (int i = 1; i < argc; i++) {
    cout << "Loading file: " << argv[i] << endl;

    // check if file exists
    if (!fs::exists(argv[i])) {
      cerr << "file " << argv[i] << " does not exist" << endl;
      continue;
    }

    auto s = imp.importSTEP(argv[i]);

    if (s == std::nullopt) {

    }
/*
    if (s == IFSelect_RetVoid) {
      cerr << "No file to transfer" << endl;
    }

    if (s == IFSelect_RetError || s == IFSelect_RetFail) {
      cerr << "Error encountered loading " << argv[i] << endl;
    }
    */
  }

  auto bodies = TopTools_ListOfShape{};

  // move the shapes from HSequenceOfShape to ListofShape
  // TODO: is this necessary?
  /*
  for (auto &body : s.value()) {
    bodies.Append(body);
  }*/

  if (bodies.Size() < 1) {
    cerr << "No shapes to slice" << endl;
    return 1;
  }

  // slice the body into layers
  auto a = splitter(bodies, makeTools(1, 10));
  // if failure, exit
  if (!a) {
    return 1;
  }
  // convert each layer into GCode

  return 0;
}

void layFlat(const TopoDS_Face &face) {
    // create a face on the XY plane
    auto basePlane  = BRepBuilderAPI_MakeFace(gp_Pln(gp_Pnt(0,0,0),gp_Dir(0,0,1)));
    // measure the angle between the face and the XY plane

    auto transform = gp_Trsf{};
}

/**
 * @brief runTests
 */
void runTests() {
  auto testsDir = fs::path("res/tests/");
  if (!fs::exists(testsDir)) {
    std::cerr << "Error, directory " << testsDir << " does not exist" << endl;
    return;
  }

  for (auto &path : fs::recursive_directory_iterator(testsDir)) {
    std::cout << "slicing file: " << path << endl;
  }
}

/**
 * @brief makeTools
 * @param layerHeight
 * @param objectHeight
 * @return A list of tools (planar faces) to slice an object
 */
TopTools_ListOfShape makeTools(const double layerHeight,
                               const double objectHeight) {
  auto result = TopTools_ListOfShape{};

  for (auto i = 0; i < objectHeight / layerHeight; ++i) {
    // create an unbounded plane, parallel to the xy plane, then convert it to a
    // face
    result.Append(BRepBuilderAPI_MakeFace(
        gp_Pln(gp_Pnt(0, 0, i * layerHeight), gp_Dir(0, 0, 1))));
  }
  return result;
}

/**
 * @brief makeSpiralFace
 * @param height
 * @param radius
 * @return
 */
TopoDS_Face makeSpiralFace(const double height, const double layerheight) {
    // make a unit cylinder, vertical axis, center @ (0,0), radius of 1
    Handle_Geom_CylindricalSurface cylinder = new Geom_CylindricalSurface(gp::XOY(), 1.0);
    auto line = gp_Lin2d(gp_Pnt2d(0.0, 0.0), gp_Dir2d(layerheight, 1.0));
    Handle_Geom2d_TrimmedCurve segment = GCE2d_MakeSegment(line, 0.0, M_PI * 2.0);
    // make the helixcal edge
    auto helixEdge = BRepBuilderAPI_MakeEdge(segment, cylinder, 0.0, 6.0 * M_PI).Edge();
    auto wire = BRepBuilderAPI_MakeWire(helixEdge);
    // make infinite line to sweep
    auto profile = NULL;;
    // sweep line to create face
    auto face = GeomFill_Pipe();
    auto a = BRepOffsetAPI_MakePipe();

    return face;
}

/**
 * @brief debug_results
 * @param result
 */
void debug_results(const TopoDS_Shape &result) {
  std::cout << TopAbs::ShapeTypeToString(result.ShapeType()) << endl;

  auto it = TopoDS_Iterator(result);
  for (; it.More(); it.Next()) {
    std::cout << TopAbs::ShapeTypeToString(it.Value().ShapeType()) << endl;
    it.Value().Location();
  }
}

/**
 * @brief splitter Use the splitter algorithm to split a solid into slices
 * @param objects
 * @param tools
 * @return the list of shapes, or std::nullopt if failure
 */
std::optional<TopoDS_Shape> splitter(const TopTools_ListOfShape &objects,
                                     const TopTools_ListOfShape &tools) {
  auto splitter = BRepAlgoAPI_Splitter{};
  // set the argument
  splitter.SetArguments(objects);
  splitter.SetTools(tools);
  // run in parallel
  splitter.SetRunParallel(true);
  splitter.SetFuzzyValue(0.0);
  // run the algorithm
  splitter.Build();
  // check error status
  if (splitter.HasErrors()) {
    cerr << "Error while splitting shape" << endl;
    splitter.DumpErrors(cerr);
    return std::nullopt;
  }

  // result of the operation result
  auto &result = splitter.Shape();
  debug_results(result);
  return result;
}

/**
 * @brief section use the section algorithm to obtain a list of edges from an
 * intersection
 * @param objects
 * @param tools
 */
void section(const TopTools_ListOfShape &objects,
             const TopTools_ListOfShape &tools) {
  BOPAlgo_Section section;
  // get first object
  TopoDS_Shape object = objects.First();

  TopTools_ListOfShape result;

  for (auto face : tools) {
    // section object
    section.AddArgument(object);
    section.AddArgument(face);
    //section.BuildSection();
    result = section.Generated(object);
  }
}

/**
 * @brief wires
 * @param s
 * @param f
 */
void wires(const TopoDS_Shape &s, const TopoDS_Face &f) {
  // TopoDS_Wire w;
  // BRepTools_WireExplorer e;
  // find all faces in solid
  auto explore_faces = TopExp_Explorer{};

  // find faces coincident with slicing plane
  for (explore_faces.Init(s, TopAbs_FACE); explore_faces.More();
       explore_faces.Next()) {
  }
}
