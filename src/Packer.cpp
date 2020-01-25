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
 * Based on this excellent article https://codeincomplete.com/posts/bin-packing/
 *
 * @author Karl Nilsson
 */

#include <sse/Packer.hpp>

namespace sse {

Node::Node(double x, double y, double w, double l)
    : x(x), y(y), width(w), length(l) {
  // ensure both children are null
  up = nullptr;
  right = nullptr;
  // initialize with no object
  object = nullptr;
}

Node* Node::search(std::shared_ptr<Object> o) {
  // leaf nodes are the only nodes valid for object insertion. this also
  // prevents inserting objects in the extraneous root nodes created when
  // growing the bin
  if (leaf()) {
    // if node is full, or is too small, return
    if (full() || !fits(o)) {
      return nullptr;
    }
    // return suitable node
    return this;
  } else {
    // search the right child
    auto n = right->search(o);
    // otherwise, try up child
    return (n ? n : up->search(o));
  }
}

void Node::add_object(std::shared_ptr<Object> o) {
  object = o;
  // add child to the right of the inserted object
  right = std::make_shared<Node>(x + o->width(), y, width - o->width(), length);
  // add child above the inserted object
  up = std::make_shared<Node>(x, y + o->length(), width, length - o->length());
}

void Node::add_children(node_ptr up, node_ptr right) {
  this->up = up;
  this->right = right;
}

void Node::translate(double offset_x, double offset_y) {
  // only translate if the node has an object
  if (full()) {
    // calculate translation dimensions
    auto delta_x = x - object->get_bound_box().CornerMin().X() + offset_x;
    auto delta_y = y - object->get_bound_box().CornerMin().Y() + offset_y;
    // perform translation
    object->translate(delta_x, delta_y, 0);
  }
  // recurse to children
  if (!leaf()) {
    up->translate(offset_x, offset_y);
    right->translate(offset_x, offset_y);
  }
}

Packer::Packer(std::vector<std::shared_ptr<Object>> objects)
    : objects(objects) {
  // sort the objects, biggest to smallest, in terms of footprint
  // specifically, compare the largest dimension (X or Y) of each object
  spdlog::debug("BinPack: sorting object list");
  std::sort(objects.begin(), objects.end(), [](std::shared_ptr<Object> lhs, std::shared_ptr<Object> rhs) {
    return std::max(lhs->length(), lhs->width()) >
           std::max(rhs->length(), rhs->width());
  });
  // create the root node. with dimensions equal to the first object
  // this is essential, so that we don't have to grow the bin in two dimensions
  // simultaneously
  spdlog::debug("BinPack: creating root node");
  root = std::make_shared<Node>(0, 0, objects.front()->width() + OFFSET,
                                objects.front()->length() + OFFSET);
}

void Packer::grow_up(double w, double l) {
  // TODO: dynamic, object-specific offset
  w += OFFSET;
  l += OFFSET;
  // create new root node
  auto new_root = std::make_shared<Node>(0, 0, root->width, root->length + l);
  // create child node
  auto up_child = std::make_shared<Node>(0, root->length, w, l);
  // other child is the previous root node
  new_root->add_children(up_child, root);
  // finally, change pointer to the new root
  root = new_root;
}

void Packer::grow_right(double w, double l) {
  // TODO: dynamic, object-specific offset
  w += OFFSET;
  l += OFFSET;
  // create new root node
  auto new_root = std::make_shared<Node>(0, 0, root->width + w, root->length);
  // create child node
  auto right_child = std::make_shared<Node>(root->width, 0, w, l);
  // other child is the previous root node
  new_root->add_children(root, right_child);
  // finally, change pointer to the new root
  root = new_root;
}

std::pair<double, double> Packer::pack() {
  spdlog::debug("BinPack: packing");
  // insert all objects into the tree
  for (auto o : objects) {
    spdlog::debug("BinPack: searching for suitable node");
    // attempt to find a suitable node for the object
    auto result = root->search(o);
    // no node found, so grow bin
    if (!result) {
      spdlog::debug("BinPack: insufficient space; growing node");
      // determine which direction to grow
      auto can_grow_up = o->width() < root->width;
      auto should_grow_up =
          can_grow_up && (root->length >= (root->width + o->width()));
      auto can_grow_right = o->length() < root->length;
      auto should_grow_right =
          can_grow_right && (root->width >= (root->leaf() + o->length()));

      // grow the bin in the correct direction
      if (should_grow_right) {
        grow_right(o->width(), o->length());
      } else if (should_grow_up) {
        grow_up(o->width(), o->length());
      } else if (can_grow_right) {
        grow_right(o->width(), o->length());
      } else if (can_grow_up) {
        grow_up(o->width(), o->length());
      } else {
        // if we can't determine which direction to grow, throw an error
        spdlog::error(
            "BinPack Error: Can't determine correct growth direction of bin");
        throw std::runtime_error(
            "BinPack: Can't determine correct growth direction of bin");
      }

      // retry search
      result = root->search(o);
      // if search failed after growing, something is wrong, throw error
      if (!result) {
        spdlog::error("BinPack error: failed to fit object after resize");
        throw std::runtime_error(
            "Bin Packing error: failed to fit object after growing node");
      }
    }
    // print object dimensions and location in bin
    spdlog::debug("BinPack: adding object to tree: {.3f}x{.3f} @ ({.3f},{.3f})",
                  o->width(), o->length(), result->x, result->y);
    // add object to suitable node
    result->add_object(o);
  }

  // return the dimensions of the bin: (width, length)
  return std::make_pair(root->width, root->length);
}

void Packer::translate(double offset_x, double offset_y) {
  spdlog::debug("BinPack: translating objects to new location");
  root->translate(offset_x, offset_y);
}

} // namespace sse
