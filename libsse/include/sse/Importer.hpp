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

#pragma once

#include <IGESControl_Reader.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <IFSelect.hxx>
#include <IFSelect_PrintCount.hxx>
#include <Standard.hxx>
#include <Standard_CString.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <Interface_Static.hxx>
#include <BRepBuilderAPI.hxx>
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>


#include <iostream>
#include <string>
#include <algorithm>

namespace sse {

class Importer {
public:
//  Importer();

  TopoDS_Shape import(const std::string &filename);

  /**
   * @brief Import STEP file
   * @param filename
   * @return TopoDS_Shape if successful
   */
  TopoDS_Shape
  importSTEP(const std::string &filename);


  TopoDS_Shape importIGES(const std::string &filename);

  TopoDS_Shape importSolid(const std::string &filename, const bool STEP);

  TopoDS_Shape importMesh(const std::string &filename);

  /**
   * @brief importBREP
   * @param filename
   * @return
   */
  TopoDS_Shape importBREP(const std::string &filename);
};


} // namespace sse
