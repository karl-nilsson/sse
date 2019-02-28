#include <iostream>

#include "importer.h"

#include <IFSelect.hxx>
#include <IFSelect_PrintCount.hxx>
#include <STEPControl_Reader.hxx>
#include <Standard.hxx>

/**
 * @brief Importer::Importer
 */
Importer::Importer() {}

std::optional<TopoDS_Shape> Importer::importSTEP(const Standard_CString &filename) {

  auto reader = STEPControl_Reader{};
  auto status = reader.ReadFile(filename);
  if (status != IFSelect_RetDone) {
    return std::nullopt;
  }

  // increase default trace level
  // reader.WS()->MapReader()->SetTraceLevel(2);

  // check the file
  reader.PrintCheckLoad(false, IFSelect_ItemsByEntity);

  // Root transfers
  auto nbr = reader.NbRootsForTransfer();
  reader.PrintCheckTransfer(Standard_False, IFSelect_ItemsByEntity);

  reader.TransferRoots();
  std::optional<TopoDS_Shape> shapes = reader.OneShape();

  return shapes;
}

IFSelect_ReturnStatus Importer::importIGES(const Standard_CString &filename,
                                           Handle(TopTools_HSequenceOfShape) &
                                               shapes) {
  return IFSelect_RetVoid;
}

IFSelect_ReturnStatus Importer::importMesh(const std::string &filename,
                                           Handle(TopTools_HSequenceOfShape) &
                                               shapes) {
  return IFSelect_RetVoid;
}
