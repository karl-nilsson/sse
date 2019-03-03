#pragma once

#include <IGESControl_Reader.hxx>
#include <STEPControl_Reader.hxx>

#include <Standard_CString.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <optional>

class Importer {
public:
  Importer();

  const std::optional<TopoDS_Shape> importSTEP(const Standard_CString &filename);
  const IFSelect_ReturnStatus importIGES(const std::string &filename,
                                   Handle(TopTools_HSequenceOfShape) & shapes);

  const IFSelect_ReturnStatus importMesh(const std::string &filename,
                                   Handle(TopTools_HSequenceOfShape) &shapes);
};
