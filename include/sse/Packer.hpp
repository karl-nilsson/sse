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
 * @file Packer.hpp
 * @brief Packs a objects into a rectangular bin
 *
 * This contains the prototypes for the Packer class
 *
 * @author Karl Nilsson
 * @bug fixed/static offset dimension between objects
 */

#pragma once

#include <algorithm>
#include <exception>
#include <vector>
#include <memory>

#include <sse/Object.hpp>

#include <spdlog/spdlog.h>

/**
 * TODO:
 * allow for configurable/dynamic OFFSET space between footprint rectangles
 * (i.e. expand all rectangles individually, based on brim, if applicable.
 * keep in mind, brim may not expand footprint, i.e. brim for a sphere)
 */
/// buffer space 10mm each direction
#define OFFSET 10

namespace sse {

/**
 * @class Node
 * @brief Binary tree Node
 *
 * This class describes a binary tree Node that corresponds to a rectangle in
 * the cartesian plane. The tree is used to pack objects into a rectangular bin.
 * For the purposes of this class, width is a dimension in the X axis, and
 * length in the Y axis.
 */
class Node {
  //! shorthand for shared_ptr
  using node_ptr = std::shared_ptr<Node>;

public:
  //! X position
  const double x;
  //! Y position
  const double y;
  //! node width
  const double width;
  //! node length
  const double length;

  /**
   * @brief Node constructor
   * @param y X position
   * @param y Y position
   * @param w Width, X axis
   * @param l Length, Y axis
   */
  Node(double x, double y, double w, double l);

  /**
   * @brief Check to see if object will fit in this node
   * @param o Target object
   * @return Whether object fits in node
   */
  inline bool fits(std::shared_ptr<Object> o) {
    return (o->length() + OFFSET < length) && (o->width() + OFFSET < width);
  }

  /**
   * @brief Does this node contain an object?
   * @return Whether node contains an object
   */
  inline bool full() { return object != nullptr; }

  /**
   * @brief is this node a leaf?
   * @return Whether node is a leaf
   */
  inline bool leaf() { return up != nullptr; }

  /**
   * @brief Search the tree for a suitable node to hold an object
   * @param o The object to insert
   * @return pointer to a suitable node, nullptr otherwise
   */
  Node* search(std::shared_ptr<Object> o);

  /**
   * @brief Add object to node, then split remaining space into 2 child nodes
   * @param o Object to be inserted
   */
  void add_object(std::shared_ptr<Object> o);

  /**
   * @brief Translate object, then recurse to children
   * @param offset_x X offset of bin with respect to buildplate origin
   * @param offset_y Y offset of bin with respect to buildplate origin
   */
  void translate(double offset_x, double offset_y);

  /**
   * @brief Add child nodes
   * @param up Up child
   * @param right Right child
   */
  void add_children(node_ptr up, node_ptr right);

private:
  //! up child node
  node_ptr up;
  //! right child node
  node_ptr right;
  //! object contained in this node
  std::shared_ptr<Object> object;
};

/**
 * @class Packer
 * @brief Pack objects into a rectangular bin, based on their XY bounding box.
 *
 * Construct a binary tree of Nodes, growing the bin and tree to fit objects. A
 * few heuristics are employed to minimize wasted space, ensuring the resulting
 * bin has close to equal dimensions (i.e. roughly a square).
 *
 */
class Packer {

public:
  /**
   * @brief Packer constructor
   * @param objects List of objects to pack
   */
  Packer(std::vector<std::shared_ptr<Object>> objects);

  /**
   * @brief Calculate an optimized rectangular bin for the objects
   * @return Dimensions of resulting bin
   * @throws std::runtime Thrown if can't grow bin properly, or can't insert object after growing bin
   */
  std::pair<double, double> pack();

  /**
   * @brief Translate objects to their new position
   * @param offset_x X offset of bin with respect to buildplate origin
   * @param offset_y Y offset of bin with respect to buildplate origin
   */
  void translate(double offset_x, double offset_y);

private:
  /**
   * @brief Grow the bin in the +Y direction
   * @param w Width requested
   * @param l Length requested
   */
  void grow_up(double w, double l);

  /**
   * @brief Grow the bin in the +X direction
   * @param w Width requested
   * @param l Length requested
   */
  void grow_right(double w, double l);

  //! list of objects to pack
  std::vector<std::shared_ptr<Object>> objects;
  //! root node of binary tree
  std::shared_ptr<Node> root;
};

} // namespace sse
