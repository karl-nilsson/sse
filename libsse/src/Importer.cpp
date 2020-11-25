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

/**
 * @brief
 *
 * @author Karl Nilsson
 * @todo refactor, DRY
 *
 */


// std headers
#include <ctype.h>
#include <algorithm>
#include <stdexcept>
// OCCT headers
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <IFSelect_PrintCount.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <IGESControl_Reader.hxx>
#include <STEPControl_Reader.hxx>
// project headers
#include <sse/Importer.hpp>


namespace sse {

/**
 * @brief Importer::Importer
 */
TopoDS_Shape Importer::import(const std::string &filename) {
  // get the file extension
  const auto i = filename.rfind('.', filename.length());
  if (i == std::string::npos) {
    throw std::runtime_error("Error: filename missing extension: " + filename);
  }
  std::string extension = filename.substr(i + 1, filename.length() - 1);
  // convert extension to lowercase
  std::transform(extension.begin(), extension.end(), extension.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  // TODO: refacto, DRY
  if (extension == "step") {
    return importSTEP(filename);
  } else if (extension == "iges") {
    return importIGES(filename);
  } else if (extension == "brep") {
    return importBREP(filename);
  } else if (extension == "stl") {
    return importMesh(filename);
  } else if (extension == "obj") {
    return importMesh(filename);
  } else if (extension == "stepz") {
    // unzip file
    // process temporary file
    return importSTEP("");
  } else {
    throw std::runtime_error("Error: invalid file: " + filename);
  }
}

TopoDS_Shape Importer::importSTEP(const std::string &filename) {

  auto reader = STEPControl_Reader();
  auto status = reader.ReadFile(filename.c_str());
  // debug info
  reader.PrintCheckLoad(false, IFSelect_ListByItem);
  // return error
  if (status != IFSelect_RetDone) {
    throw std::runtime_error("Error: importing file failed: " + filename);
    // return status;
  }

  // increase default trace level
  // reader.WS()->MapReader()->SetTraceLevel(2);

  // check the file
  reader.PrintCheckLoad(false, IFSelect_ItemsByEntity);

  // Root transfers
  auto nbr = reader.NbRootsForTransfer();
  reader.PrintCheckTransfer(false, IFSelect_ItemsByEntity);

  reader.TransferRoots();

  return reader.OneShape();
}

TopoDS_Shape Importer::importIGES(const std::string &filename) {
  auto reader = IGESControl_Reader();
  auto status = reader.ReadFile(filename.c_str());
  // debug info
  reader.PrintCheckLoad(false, IFSelect_ListByItem);
  if (status != IFSelect_RetDone) {
    throw std::runtime_error("Error: importing file failed: " + filename);
  }
  reader.PrintCheckLoad(false, IFSelect_ItemsByEntity);
  auto nbr = reader.NbRootsForTransfer();
  reader.PrintCheckTransfer(false, IFSelect_ItemsByEntity);
  reader.TransferRoots();
  return reader.OneShape();
}

TopoDS_Shape Importer::importSolid(const std::string &filename, const bool STEP) {
  // FIXME
  auto reader = STEPControl_Reader();
  auto status = reader.ReadFile(filename.c_str());
  // debug info
  reader.PrintCheckLoad(false, IFSelect_ListByItem);
  // return error
  if (status != IFSelect_RetDone) {
    throw std::runtime_error("Error: importing file failed: " + filename);
    // return status;
  }

  // increase default trace level
  // reader.WS()->MapReader()->SetTraceLevel(2);

  // check the file
  reader.PrintCheckLoad(false, IFSelect_ItemsByEntity);

  // Root transfers
  // auto nbr = reader.NbRootsForTransfer();
  reader.PrintCheckTransfer(false, IFSelect_ItemsByEntity);

  reader.TransferRoots();

  return reader.OneShape();
}

TopoDS_Shape Importer::importMesh(const std::string &filename) {
  return TopoDS_Shape();
}

TopoDS_Shape Importer::importBREP(const std::string &filename) {
  TopoDS_Shape shape;
  BRep_Builder b;
  BRepTools::Read(shape, filename.c_str(), b);
  return TopoDS_Shape();
}

} // namespace sse
