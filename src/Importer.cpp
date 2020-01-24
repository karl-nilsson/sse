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

#include <sse/Importer.hpp>

namespace sse {

/**
 * @brief Importer::Importer
 */
Importer::Importer() {}

const std::optional<TopoDS_Shape>
Importer::importSTEP(const Standard_CString &filename) {

  auto reader = STEPControl_Reader();
  auto status = reader.ReadFile(filename);
  // debug info
  reader.PrintCheckLoad(false, IFSelect_ListByItem);
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

} // namespace sse
