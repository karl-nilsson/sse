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
 * @file Rearrange.cpp
 * @brief Rearrange objects to minimize the aggregate footprint
 *
 * Construct a binary tree of Nodes, growing the bin and tree to fit objects. A
 * few heuristics are employed to minimize wasted space, ensuring the resulting
 * bin has close to equal dimensions (i.e. roughly a square).
 *
 * Inspired by this excellent article
 * https://codeincomplete.com/posts/bin-packing/
 *
 * @author Karl Nilsson
 */

/**
 * TODO:
 * allow for configurable/dynamic OFFSET space between footprint rectangles
 * (i.e. expand all rectangles individually, based on brim, if applicable.
 * keep in mind, brim may not expand footprint, i.e. brim for a sphere)
 */

// std headers
#include <algorithm>
#include <exception>
#include <memory>
#include <utility>
#include <vector>
// OCCT headers
#include <Standard_Version.hxx>
// external headers
#include <spdlog/spdlog.h>
// project headers
#include "sse/slicer.hpp"


struct Node;

using node_ptr = std::unique_ptr<Node>;

/**
 * @struct Node
 * @brief Binary tree Node
 *
 * This struct describes a binary tree Node that corresponds to a rectangle in
 * the cartesian plane. The tree is used to rearrange objects into a rectangular
 * bin. For the purposes of this struct, width and length are the dimensions of
 * the X and Y axes, respectively
 */
struct [[nodiscard]] Node {
  //! X position
  const double x;
  //! Y position
  const double y;
  //! node width
  const double width;
  //! node length
  const double length;
  //! up child node
  node_ptr up{nullptr};
  //! right child node
  node_ptr right{nullptr};
  //! object contained in this node
  sse::Object *object{nullptr};

  /**
   * @brief Node constructor
   * @param y X position
   * @param y Y position
   * @param w Width, X axis
   * @param l Length, Y axis
   */
  Node(double x, double y, double w, double l, node_ptr up = nullptr,
       node_ptr right = nullptr)
      : x{x}, y{y}, width{w}, length{l}, up{std::move(up)},
          right{std::move(right)} {}

  /**
   * @brief Add object to node, then make child nodes out of leftovers
   * @param o Object to add
   */
  void add_object(sse::Object *o) {
    object = o;
    up = std::make_unique<Node>(x, y + object->length(), width,
                                length - object->length());
    right = std::make_unique<Node>(x + object->length(), y,
                                   width - object->width(), length);
  }

  /**
   * @brief Check to see if object will fit in this node
   * @param o Target object
   * @return Whether object fits in node
   */
  [[nodiscard]] inline bool fits(const sse::Object *o) const {
    return (o->length() <= length) && (o->width() <= width);
  }

  /**
   * @brief Does this node contain an object?
   * @return Whether node contains an object
   */
  [[nodiscard]] inline bool full() const { return object != nullptr; }

  /**
   * @brief Is this node a leaf?
   *
   * n.b. child nodes are always created in pairs, so it's only necessary to
   * check one of the children
   *
   * @return Whether node is a leaf
   */
  [[nodiscard]] inline bool leaf() const { return up == nullptr; }

}; // end Node definition

/**
 * @brief Recursively search the tree for a suitable node to hold the object
 * @param o The object to insert
 * @return pointer to a suitable node, nullptr if none found
 */
[[nodiscard]] static Node *insert_search(Node &node, const sse::Object *o) {
  // only leaf nodes are valid candidates for object insertion. this also
  // prevents inserting objects in the extraneous root nodes created when
  // growing the bin
  if (node.leaf()) {
    // if node is full, or is too small, return failure
    if (node.full() || !node.fits(o)) {
      return nullptr;
    }
    // success
    return &node;
  }
  // node is not a leaf, search children
  auto *n = insert_search(*node.right, o);
  return (n != nullptr ? n : insert_search(*node.up, o));
}

/**
 * @brief Grow the bin in the +Y direction
 * @param length Length requested
 * @return A new Node, big enough to fit the space required
 */
[[nodiscard]] static Node *grow_up(node_ptr &root, const double length) {
  // create a larger root node
  // up child: new node of desired length, located above old root node
  // right child: previous root node
  root = std::make_unique<Node>(
      0, 0, root->width, root->length + length,
      std::make_unique<Node>(0, root->length, root->width, length),
      std::move(root));
  // return available node
  return root->up.get();
}

/**
 * @brief Grow the bin in the +X direction
 * @param width Width requested
 * @return A new Node, big enough to fit the space required
 */
[[nodiscard]] static Node *grow_right(node_ptr &root, const double width) {
  // create a larger root node
  // up child: previous root node
  // right child: new node of desired width, located right of old root node
  root = std::make_unique<Node>(
      0, 0, root->width + width, root->length, std::move(root),
      std::make_unique<Node>(root->width, 0, width, root->length));
  // return available node
  return root->right.get();
}

/**
 * @brief Translate an object to its new position, then recurse to children
 * @param node Object to translate
 * @param offset_x X offset of bin with respect to buildplate origin
 * @param offset_y Y offset of bin with respect to buildplate origin
 */
static void translate(const Node &node, const double offset_x,
                      const double offset_y) {
  // only translate if the node has an object
  if (node.full()) {
    spdlog::debug("Rearrange: moving object to ({:.3f},{:.3f})",
                  node.x + offset_x, node.y + offset_y);
    // perform translation (only in XY direction)
    node.object->translate(
        gp_Pnt(node.x + offset_x, node.y + offset_y,
               node.object->get_bound_box().CornerMin().Z()));
  }
  // recurse to children
  if (!node.leaf()) {
    translate(*node.up, offset_x, offset_y);
    translate(*node.right, offset_x, offset_y);
  }
}

void sse::rearrange_objects(std::vector<std::unique_ptr<sse::Object>> &objects,
                            const double bed_width, const double bed_length) {
  spdlog::info("Rearranging objects");

  // short-circuit if no objects
  if (objects.empty()) {
    spdlog::warn("Rearrange: attempting to rearrange zero objects");
    return;
  }

  // negative bed dimensions are invalid
  if (bed_width <= 0 || bed_length <= 0) {
    spdlog::error("Rearrange: bed dimensions cannot be < 0");
    throw std::invalid_argument("Rearrange: invalid bed dimensions");
  }

  // check for excessive input size
  if (objects.size() > SSE_MAXIMUM_NUM_OBJECTS) {
    spdlog::error("Rearrange: object count {} exceeds maximum {}",
                  objects.size(), SSE_MAXIMUM_NUM_OBJECTS);
    throw std::invalid_argument("Rearrange: Too many objects");
  }

  // need to sort the input, so make a copy (avoid side effects)
  auto objects_list = std::vector<Object*>();
  objects_list.reserve(objects.size());

  // check for invalid objects
  for (const auto &o : objects) {
#if OCC_VERSION_HEX >= 0x070400
    if (o->get_bound_box().IsOpen()) {
#else
    // clang-format off
    if (o->get_bound_box().IsOpenXmin() || o->get_bound_box().IsOpenXmax() ||
        o->get_bound_box().IsOpenYmin() || o->get_bound_box().IsOpenYmax() ||
        o->get_bound_box().IsOpenZmin() || o->get_bound_box().IsOpenZmax()) {
    // clang-format on
#endif
      spdlog::error("Rearrange: Error: object with infinite dimension");
      throw std::invalid_argument("Rearrange: object has infinite volume");
    }

    if (o->get_bound_box().IsVoid()) {
      spdlog::error("Rearrange: Error: empty object");
      throw std::invalid_argument("Rearrange: object is empty");
    }

    if (o->width() > bed_width || o->length() > bed_length) {
      spdlog::error("Rearrange: Error: object ({:.3f}x{:.3f}) too large for "
                    "bed ({:.3f}x{:.3f})",
                    o->width(), o->length(), bed_width, bed_length);
      throw std::invalid_argument("Rearrange: object too large for bed");
    }

    objects_list.push_back(o.get());
  }

  // sort the objects, largest to smallest, in terms of footprint
  // specifically, compare the largest dimension (X or Y) of each object
  spdlog::trace("Rearrange: sorting object list");
  std::sort(objects_list.begin(), objects_list.end(),
            [](const auto &lhs, const auto &rhs) {
              return std::max(lhs->length(), lhs->width()) >
                     std::max(rhs->length(), rhs->width());
            });

  // create the root node, with dimensions equal to the first object
  // this is essential, to avoid growing the bin in two dimensions
  // simultaneously
  spdlog::trace(
      "Rearrange: creating root node: {:.3f}x{:.3f} @ ({:.3f},{:.3f})",
      objects_list.front()->width(), objects_list.front()->length(), 0.0, 0.0);
  node_ptr root = std::make_unique<Node>(0, 0, objects_list.front()->width(),
                                         objects_list.front()->length());

  spdlog::trace("Rearrange: starting");
  // insert all objects into the tree
  for (const auto o : objects_list) {
    // attempt to find a suitable node for the object
    spdlog::trace("Rearrange: searching for suitable node");
    auto *result = insert_search(*root, o);
    // if no node found, grow the bin
    if (result == nullptr) {
      spdlog::trace("Rearrange: insufficient space; growing bin");
      // determine which direction to grow
      auto can_grow_up = o->width() <= root->width;
      auto can_grow_right = o->length() <= root->length;
      auto should_grow_up =
          can_grow_up && (root->width >= (root->length + o->length()));
      auto should_grow_right =
          can_grow_right && (root->length >= (root->width + o->width()));

      // grow the bin in the correct direction
      if (should_grow_right) {
        result = grow_right(root, o->width());
      } else if (should_grow_up) {
        result = grow_up(root, o->length());
      } else if (can_grow_right) {
        result = grow_right(root, o->width());
      } else if (can_grow_up) {
        result = grow_up(root, o->length());
      } else {
        // the only way to reach this codepath is if the object is changed underneath us
        spdlog::error(
            "Rearrange Error: Can't determine correct growth direction of bin");
        throw std::runtime_error(
            "Rearrange: Can't determine correct growth direction of bin");
      }
    }

    // add object to suitable node
    spdlog::trace(
        "Rearrange: adding object to bin: {:.3f}x{:.3f} @ ({:.3f},{:.3f})",
        o->width(), o->length(), result->x, result->y);
    result->add_object(o);
  }

  // after inserting all objects, the dimensions of the root node will encompass
  // all objects. Check its boundaries to ensure it will fit the build volume
  if (root->width > bed_width || root->length > bed_length) {
    spdlog::error(
        "Rearrange: objects {:.3f}x{:.3f} exceed bed area {:.3f}x{:.3f}",
        root->width, root->length, bed_width, bed_length);
    throw std::runtime_error("Objects too large to fit onto bed");
  }

  spdlog::trace("Rearrange: final bin size: {:.3f}x{:.3f}", root->width,
                root->length);

  auto offset_x = (bed_width - root->width) / 2;
  auto offset_y = (bed_length - root->length) / 2;

  spdlog::debug(
      "Rearrange: translating objects to new location, offset ({:.3f},{:.3f})",
      offset_x, offset_y);

  translate(*root, offset_x, offset_y);
}
