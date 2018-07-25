#ifndef IMPORTER_H
#define IMPORTER_H

#include <IGESControl_Reader.hxx>
#include <STEPControl_Reader.hxx>

#include <Standard_CString.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <TopTools_ListOfShape.hxx>

class Importer {
public:
  Importer();

  IFSelect_ReturnStatus importSTEP(const Standard_CString &filename,
                                   Handle(TopTools_HSequenceOfShape) & shapes);
  IFSelect_ReturnStatus importIGES(const Standard_CString &filename,
                                   Handle(TopTools_HSequenceOfShape) & shapes);
};

#endif // IMPORTER_H
