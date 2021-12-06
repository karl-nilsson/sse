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
 * @brief Import files into occt TopoDS_Shape's
 *
 * @author Karl Nilsson
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
#include "sse/slicer.hpp"


static TopoDS_Shape importSTEP(const std::string &filename) {

  auto reader = STEPControl_Reader();
  auto status = reader.ReadFile(filename.c_str());
  // return error
  if (status != IFSelect_RetDone) {
    spdlog::error("Importer: failed to read file: {}", filename);
    throw std::runtime_error("Importer: failed to read file: " + filename);
  }

  // increase default trace level
  // reader.WS()->MapReader()->SetTraceLevel(2);

  // Root transfers
  auto nbr = reader.NbRootsForTransfer();

  reader.TransferRoots();

  return reader.OneShape();
}

static TopoDS_Shape importIGES(const std::string &filename) {
  auto reader = IGESControl_Reader();
  auto status = reader.ReadFile(filename.c_str());

  if (status != IFSelect_RetDone) {
    spdlog::error("Importer: failed to read file: {}", filename);
    throw std::runtime_error("Iporter: failed to read file: " + filename);
  }
  auto nbr = reader.NbRootsForTransfer();
  reader.TransferRoots();
  return reader.OneShape();
}

static TopoDS_Shape importMesh(const std::string &filename) {
  return {};
}

static TopoDS_Shape importBREP(const std::string &filename) {
  TopoDS_Shape shape;
  BRep_Builder b;
  auto status = BRepTools::Read(shape, filename.c_str(), b);
  if(!status) {
    spdlog::error("Importer: failed to read file: {}", filename);
    throw std::runtime_error("Importer: failed to read file: " + filename);
  }
  return shape;
}

namespace sse {

TopoDS_Shape import(const std::string &filename) {
  if(filename.empty()) {
    spdlog::error("Importer: empty filename given");
    throw std::invalid_argument("Importer: empty filename provided");
  }

  if(!fs::exists(filename)) {
    spdlog::error("Importer: file does not exist: {}", filename);
    throw std::invalid_argument("Importer: file does not exist: " + filename);
  }

  // get the file extension
  const auto i = filename.rfind('.', filename.length());
  if (i == std::string::npos) {
    spdlog::error("Importer: filename missing extension: {}", filename);
    throw std::invalid_argument("Importer: filename missing extension: " + filename);
  }
  std::string extension = filename.substr(i + 1, filename.length() - 1);

  // convert extension to lowercase
  std::transform(extension.begin(), extension.end(), extension.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  spdlog::trace("Importer: Filename: {}, Extension: {}", filename, extension);

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
  } else if (extension == "stepz" || extension == "stpz") {
    // unzip file
    // process temporary file
    return {};
  } else {
    spdlog::error("Importer: invalid file extension: {}", extension);
    throw std::invalid_argument("Invalid file extension: " + extension);
  }
}

} // namespace sse
