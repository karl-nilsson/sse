

#include <sse/Importer.hpp>

/**
 * @brief Importer::Importer
 */
Importer::Importer() {}

const std::optional<TopoDS_Shape>
Importer::importSTEP(const Standard_CString &filename) {

  auto reader = STEPControl_Reader{};
  auto status = reader.ReadFile(filename);
  // return error
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

const IFSelect_ReturnStatus
Importer::importIGES(const std::string &filename,
                     Handle(TopTools_HSequenceOfShape) & shapes) {
  return IFSelect_RetVoid;
}

const IFSelect_ReturnStatus
Importer::importMesh(const std::string &filename,
                     Handle(TopTools_HSequenceOfShape) & shapes) {
  return IFSelect_RetVoid;
}
