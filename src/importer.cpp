#include "importer.h"

Importer::Importer() {}

IFSelect_ReturnStatus Importer::importSTEP(const Standard_CString &filename,
                                           Handle(TopTools_HSequenceOfShape) &
                                               shapes) {
  shapes->Clear();

  STEPControl_Reader aReader;
  IFSelect_ReturnStatus status = aReader.ReadFile(filename);
  if (status != IFSelect_RetDone)
    return status;

  // aReader.WS()->MapReader()->SetTraceLevel(2); // increase default trace
  // level

  aReader.PrintCheckLoad(Standard_True, IFSelect_PrintCount);

  // Root transfers
  Standard_Integer nbr = aReader.NbRootsForTransfer();
  aReader.PrintCheckTransfer(Standard_True, IFSelect_ItemsByEntity);

  for (Standard_Integer n = 1; n <= nbr; n++) {
    Standard_Boolean ok = aReader.TransferRoot(n);
  }

  // Collecting resulting entities
  Standard_Integer nbs = aReader.NbShapes();
  if (nbs == 0) {
    return IFSelect_RetVoid;
  }
  for (Standard_Integer i = 1; i <= nbs; i++) {
    shapes->Append(aReader.Shape(i));
  }

  return status;
}

IFSelect_ReturnStatus Importer::importIGES(const Standard_CString &filename,
                                           Handle(TopTools_HSequenceOfShape) &
                                               shapes) {
  return IFSelect_RetVoid;
}
