#include <BOPAlgo_Tools.hxx>
#include <BRepAlgo.hxx>
#include <BRepAlgoAPI_Splitter.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>
//#include <TopoDS_Shape.hxx>
#include <gp_Pln.hxx>

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

#include <BRepTools_WireExplorer.hxx>

#include <iostream>
#include <string>

#include <boost/program_options.hpp>

#include "importer.h"
#include "main.h"

#include <gflags/gflags.h>

using namespace std;

namespace po = boost::program_options;

int main(int argc, char **argv) {
  cout << "StepSlicerEngine V0.01A" << endl;

  Importer imp;
  IFSelect_ReturnStatus s;

  Handle(TopTools_HSequenceOfShape) shapes = new TopTools_HSequenceOfShape();
  shapes->Clear();

  for (int i = 1; i < argc; i++) {
    cout << "Loading file: " << argv[i] << endl;

    s = imp.importSTEP(argv[i], shapes);

    if (s == IFSelect_RetVoid) {
      cout << "No file to transfer" << endl;
    }

    if (s == IFSelect_RetError || s == IFSelect_RetFail) {
      cerr << "Error encountered loading " << argv[i] << endl;
    }
  }

  TopTools_ListOfShape bodies;

  for (auto itr = shapes->begin(); itr != shapes->end(); ++itr) {
    bodies.Append(*itr);
  }

  slice(bodies);

  return 0;
}

void slice(const TopTools_ListOfShape &objects) {
  //
  BRepAlgoAPI_Splitter splitter;
  // tools
  TopTools_ListOfShape aLSTools;

  for (int i = 0; i < 100; i++) {
    // make an unbounded splitting plane, parallel to the xy plane
    TopoDS_Face face = BRepBuilderAPI_MakeFace(
        gp_Pln(gp_Pnt(0, 0, i / 10.0), gp_Dir(0, 0, 1)));
    aLSTools.Append(face);
  }

  // set the argument
  splitter.SetArguments(objects);
  splitter.SetTools(aLSTools);
  // run in parallel
  splitter.SetRunParallel(true);

  // run the algorithm
  splitter.Build();
  // check error status
  if (splitter.HasErrors()) {
    cout << "Error while splitting shape" << endl;
    return;
  }
  // result of the operation result
  const TopoDS_Shape &result = splitter.Shape();

  // TopTools_MapOfShape map;

  map<TopAbs_ShapeEnum, string> shapetype;
  shapetype[TopAbs_COMPOUND] = "compound";
  shapetype[TopAbs_COMPSOLID] = "compsolid";
  shapetype[TopAbs_SOLID] = "solid";
  shapetype[TopAbs_SHELL] = "shell";
  shapetype[TopAbs_FACE] = "face";
  shapetype[TopAbs_WIRE] = "wire";
  shapetype[TopAbs_EDGE] = "edge";
  shapetype[TopAbs_VERTEX] = "vertex";
  shapetype[TopAbs_SHAPE] = "shape";

  //cout << shapetype[result.ShapeType()] << endl;

  // iterate over result
  TopoDS_Iterator it;
  for (it.Initialize(result); it.More(); it.Next()) {
    it.Value();
    //cout << shapetype[it.Value().ShapeType()] << endl;
  }
}

void wires(const TopoDS_Shape &s) {
  // TopoDS_Wire w;
  // BRepTools_WireExplorer e;
  // find all faces in solid
  TopExp_Explorer explore_faces;

  for(explore_faces.Init(s, TopAbs_FACE); explore_faces.More(); explore_faces.Next())
  {
  }

}
