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
 * @file Packer.cpp
 * @brief Packs objects into a rectangular bin
 *
 * Inspired by this excellent article
 * https://codeincomplete.com/posts/bin-packing/
 *
 * @author Karl Nilsson
 */

// std headers
#include <algorithm>
#include <exception>
// external headers
#include <spdlog/spdlog.h>
// project headers
#include <sse/Packer.hpp>

namespace sse {

Packer::Packer(std::vector<std::shared_ptr<Object>> objects)
    : objects(objects) {
  // check for excessive input size
  if(objects.size() > MAXIMUM_OBJECTS) {
    throw std::runtime_error("Binpack: Too many objects");
  }
  // check for invalid objects (infinite or zero volume)
  // TODO: consider using c++20 ranges
  for (const auto &o : objects) {
    // TODO: occt v7.4.0, replace with IsOpen
    // if(o->get_bound_box().IsOpen()) {
    // clang-format off
    if (o->get_bound_box().IsOpenXmin() || o->get_bound_box().IsOpenXmax() ||
        o->get_bound_box().IsOpenYmin() || o->get_bound_box().IsOpenYmax() ||
        o->get_bound_box().IsOpenZmin() || o->get_bound_box().IsOpenZmax()) {
    // clang-format on
      spdlog::error("Binpack: Error: object with infinite dimension");
      throw std::runtime_error("Binpack: object has infinite volume");
    }
    if (o->get_bound_box().IsVoid()) {
      spdlog::error("Binpack: Error: empty object");
      throw std::runtime_error("Binpack: object is empty");
    }
  }

}

std::pair<double, double> Packer::pack() {
  // short-circuit if no objects
  if(objects.empty()) {
      spdlog::warn("BinPack: attempting to pack zero objects");
      return std::make_pair(0, 0);
  }

  // sort the objects, largest to smallest, in terms of footprint
  // specifically, compare the largest dimension (X or Y) of each object
  spdlog::debug("BinPack: sorting object list");
  std::sort(objects.begin(), objects.end(),
            [](const auto &lhs, const auto &rhs) {
              return std::max(lhs->length(), lhs->width()) >
                     std::max(rhs->length(), rhs->width());
            });

  // create the root node, with dimensions equal to the first object
  // this is essential, to avoid growing the bin in two dimensions simultaneously
  spdlog::debug("BinPack: creating root node: {}x{} @ ({},{})", objects.front()->width(), objects.front()->height(), 0, 0);
  root = std::make_unique<Node>(0, 0, objects.front()->width(),
                                objects.front()->length());

  spdlog::debug("BinPack: packing");
  // insert all objects into the tree
  for (const auto &o : objects) {
    spdlog::debug("BinPack: searching for suitable node");
    // attempt to find a suitable node for the object
    auto *result = insert_search(*root, o.get());
    // no node found, grow the bin
    if (!result) {
      spdlog::debug("BinPack: insufficient space; growing bin");
      // determine which direction to grow
      auto can_grow_up = o->width() <= root->width;
      auto can_grow_right = o->length() <= root->length;
      auto should_grow_up =
          can_grow_up && (root->width >= (root->length + o->length()));
      auto should_grow_right =
          can_grow_right && (root->length >= (root->width + o->width()));

      // grow the bin in the correct direction
      if (should_grow_right) {
        result = grow_right(o->width());
      } else if (should_grow_up) {
        result = grow_up(o->length());
      } else if (can_grow_right) {
        result = grow_right(o->width());
      } else if (can_grow_up) {
        result = grow_up(o->length());
      } else {
        // if we can't determine which direction to grow, throw an error
        // this is only possible if the object is changed underneath us
        // set root node null, to prevent a call to arrange()
        root = nullptr;
        spdlog::error(
            "BinPack Error: Can't determine correct growth direction of bin");
        throw std::runtime_error(
            "BinPack: Can't determine correct growth direction of bin");
      }
    }
    // print object dimensions and location in bin
    spdlog::debug("BinPack: adding object to bin: {}x{} @ ({},{})",
                  o->width(), o->length(), result->x, result->y);
    // add object to suitable node
    result->add_object(o.get());
  }

  // return the final dimensions of the bin: (width, length)
  return std::make_pair(root->width, root->length);
}

Packer::Node *Packer::insert_search(Node &node, const Object *o) const {
  // only leaf nodes are valid for object insertion. this also prevents
  // inserting objects in the extraneous root nodes created when
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
  return (n ? n : insert_search(*node.up, o));
}

Packer::Node *Packer::grow_up(double length) {
  // create a larger root node
  // up child node: new node of desired length, located above old root node
  // right child: previous root node
  root = std::make_unique<Node>(0,0, root->width, root->length + length,
                                std::make_unique<Node>(0, root->length, root->width, length),
                                std::move(root));
  // return available node
  return root->up.get();
}

Packer::Node *Packer::grow_right(double width) {
  // create a larger root node
  // up child: previous root node
  // right child node: new node of desired width, located right of old root node
  root = std::make_unique<Node>(0,0, root->width + width, root->length,
                                std::move(root),
                                std::make_unique<Node>(root->width, 0, width, root->length));
  // return available node
  return root->right.get();
}

void Packer::arrange(double offset_x, double offset_y) const {
  spdlog::debug("BinPack: translating objects to new location");

  if(!root) {
    spdlog::error("BinPack Error: must pack before arranging");
    return;
  }

  translate(*root, offset_x, offset_y);
}

void Packer::translate(const Node &node, const double offset_x,
                       const double offset_y) const {
  // only translate if the node has an object
  if (node.full()) {
    spdlog::debug("BinPack: moving object to ({},{})", node.x + offset_x, node.y + offset_y);
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

#undef MAXIMUM_OBJECTS

} // namespace sse
