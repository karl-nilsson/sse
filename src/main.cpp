#include <opencascade/BOPAlgo_Tools.hxx>
#include <opencascade/BRepAlgoAPI_Splitter.hxx>
#include <opencascade/BRepBuilderAPI_MakeFace.hxx>
#include <opencascade/TopTools_ListOfShape.hxx>
#include <opencascade/TopoDS_Iterator.hxx>
#include <opencascade/TopoDS_Shape.hxx>
#include <opencascade/gp_Pln.hxx>

#include <opencascade/IFSelect.hxx>
#include <opencascade/IFSelect_PrintCount.hxx>
#include <opencascade/STEPCAFControl_Reader.hxx>
#include <opencascade/STEPControl_Reader.hxx>
#include <opencascade/Standard.hxx>
#include <opencascade/Standard_CString.hxx>
#include <opencascade/TDF.hxx>
#include <opencascade/TDF_Attribute.hxx>
#include <opencascade/TopTools_HSequenceOfShape.hxx>
#include <opencascade/TopTools_SequenceOfShape.hxx>

#include <iostream>
#include <string>

#include <boost/program_options.hpp>

#include "importer.h"

using namespace std;

namespace po = boost::program_options;

int main(int argc, char **argv) {

    /*
  po::options_description desc("Convert STEP files to GCode for 3D printing");

  desc.add_options()("version,v", "print version string")(
      "help,h", "show help messag")("orientation", po::value<vector<int>>(),
                                    "translate/rotate models")(
      "config,c", po::value<string>(), "printer configuration file")(
      "profile,p", po::value<string>(), "slicing profile")(
      "input-file,i", po::value<vector<string>>(), "input file");
  po::positional_options_description p;
  p.add("input-file", -1);

  po::variables_map vm;
  po::store(
      po::command_line_parser(argc, argv).options(desc).positional(p).run(),
      vm);
  po::notify(vm);

  if (vm.count("help")) {
    cout << desc << endl;
    return 1;
  }

  if (!vm.count("input-file")) {
    cout << "must specify at least 1 input file" << endl;
    return 1;
  }


  // load STEP files
  for (auto shape_fname : vm["input-file"].as<vector<string>>()) {
    cout << shape_fname << endl;
  }
  */

  Handle(TopTools_HSequenceOfShape) shapes = new TopTools_HSequenceOfShape();
  // load_step("", shapes);

  cout << "Hello World!" << endl;
  return 0;
}

void slice(TopTools_ListOfShape &objects) {
  //
  BRepAlgoAPI_Splitter aSplitter;
  // tools
  TopTools_ListOfShape aLSTools;

  gp_Ax3 loc = gp_Ax3(gp_Pnt(0, 0, 0), gp_Dir(1, 1, 1));

  gp_Pln p = gp_Pln(loc);

  for (int i = 0; i < 100; i++) {
    TopoDS_Shape face = BRepBuilderAPI_MakeFace(p);
    aLSTools.Append(face);
  }
  // run in parallel
  Standard_Boolean bRunParallel = Standard_True;
  aSplitter.SetRunParallel(bRunParallel);
  //
  // set the argument
  aSplitter.SetArguments(objects);
  // aSplitter.SetTools(aLSTools);
  //
  // Set options for the algorithm
  // setting options for this algorithm is similar to setting options for GF
  // algorithm (see "GF Usage" chapter)
  // run the algorithm
  aSplitter.Build();
  // check error status
  if (aSplitter.HasErrors()) {
    return;
  }
  //
  // result of the operation aResult
  const TopoDS_Shape &aResult = aSplitter.Shape();
  auto it = TopoDS_Iterator(aResult, true, true);

  while (it.More()) {
  }
}
