#include "importer.h"

#include <IFSelect.hxx>
#include <IFSelect_PrintCount.hxx>
#include <Standard.hxx>

#include <iostream>

Importer::Importer() {}

IFSelect_ReturnStatus Importer::importSTEP(const Standard_CString &filename,
                                           Handle(TopTools_HSequenceOfShape) &
                                               shapes) {

  STEPControl_Reader reader;
  auto status = reader.ReadFile(filename);
  if (status != IFSelect_RetDone) {
    return status;
  }

  //reader.WS()->MapReader()->SetTraceLevel(2); // increase default trace
  // level

  reader.PrintCheckLoad(Standard_False, IFSelect_ItemsByEntity);

  // Root transfers
  auto nbr = reader.NbRootsForTransfer();
  reader.PrintCheckTransfer(Standard_False, IFSelect_ItemsByEntity);

  for (int n = 1; n <= nbr; n++) {
    auto ok = reader.TransferRoot(n);
  }

  // Collecting resulting entities
  auto nbs = reader.NbShapes();
  cout << "Number of shapes: " << nbs << endl;
  if (nbs == 0) {
    return IFSelect_RetVoid;
  }
  for (Standard_Integer i = 1; i <= nbs; i++) {
    shapes->Append(reader.Shape(i));
  }

  return status;
}

IFSelect_ReturnStatus Importer::importIGES(const Standard_CString &filename,
                                           Handle(TopTools_HSequenceOfShape) &
                                               shapes) {
  return IFSelect_RetVoid;
}
