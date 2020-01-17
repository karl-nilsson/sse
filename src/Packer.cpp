#include <sse/Packer.hpp>

namespace sse {

/**
 * @brief Node::Node Constructor
 * @param x X position
 * @param y Y position
 * @param w width, x axis
 * @param l height, y axis
 */
Node::Node(double x, double y, double w, double l) {
  this->x = x;
  this->y = y;
  this->width = w;
  this->length = l;
  // ensure both children are null
  this->up = nullptr;
  this->right = nullptr;
  // initialize with no object
  this->object = nullptr;
}

/**
 * @brief Node::insert Traverse the binary tree using depth-first-search,
 * searching for a suitable Node to hold the object
 * @param o The object to insert
 * @return pointer to a suitable node, nullptr otherwise
 */
Node *Node::search(std::shared_ptr<Object> o) {
  // leaf nodes are the only nodes valid for object insertion
  // this also prevents inserting objects in the pseudo-nodes created when
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

/**
 * @brief Add object to a Node, then split remaining space into child nodes
 * @param o Object to be inserted
 */
void Node::add_object(std::shared_ptr<Object> o) {
  object = o;
  // add child to the right of the inserted object
  right = std::make_shared<Node>(x + o->width(), y, width - o->width(), length);
  // add child above the inserted object
  up = std::make_shared<Node>(x, y + o->length(), width, length - o->length());
}

/**
 * @brief Add child nodes to a node
 * @param up Up child
 * @param right Right child
 */
void Node::add_children(node_ptr up, node_ptr right) {
  this->up = up;
  this->right = right;
}

/**
 * @brief Visit every node in the tree (DFS), translating each
 * object to its new position
 *
 */
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

/**
 * @brief Packer::Packer Constructor
 * @param objects List of Objects to pack
 */
Packer::Packer(std::vector<std::shared_ptr<Object>> objects) {
  this->objects = objects;

  // sort the objects, biggest to smallest, in terms of footprint
  // specifically, compare the largest dimension (X or Y) of each object
  spdlog::debug("BinPack: sorting objects");
  std::sort(objects.begin(), objects.end(), [](Object &lhs, Object &rhs) {
    return std::max(lhs.length(), lhs.width()) >
           std::max(rhs.length(), rhs.width());
  });
  // create the root node. with dimensions equal to the first object
  // this is essential, so that we don't have to grow the bin in two dimensions simultaneously
  spdlog::debug("BinPack: creating root node");
  root = std::make_shared<Node>(0, 0, objects.front()->width() + OFFSET,
                                objects.front()->length() + OFFSET);
}

/**
 * @brief Packer::grow_up Grow the bin in the Y-direction
 * @param root Root node
 * @param w width requested
 * @param l length requested
 */
void Packer::grow_up(double w, double l) {
  // create new root node
  auto new_root = std::make_shared<Node>(0, 0, root->width, root->length + l);
  // add child node in the new space
  auto up_child = std::make_shared<Node>(0, root->length, w, l);
  // other child is the previous root node
  new_root->add_children(up_child, root);
  // finally, change pointer to the new root
  root = new_root;
}

/**
 * @brief Packer::grow_right Grow the bin in the X-direction
 * @param root Root node
 * @param w width requested
 * @param l length requested
 */
void Packer::grow_right(double w, double l) {
  // create new root node
  auto new_root = std::make_shared<Node>(0, 0, root->width + w, root->length);
  // create a new child node,
  auto right_child = std::make_shared<Node>(root->width, 0, w, l);
  new_root->add_children(root, right_child);
  // finally, change pointer to the new root
  root = new_root;
}

/**
 * @brief Packer::pack Calculate an optimized rectangular bin for a list of
 * objects
 * @return Dimensions of resulting bin
 * @throws std::runtime
 */
std::pair<double, double> Packer::pack() {
  spdlog::debug("BinPack: packing");
  // insert all objects into the tree
  for (auto o : objects) {
    spdlog::debug("BinPack: searching for suitable node");
    // attempt to find a suitable node for the object
    auto result = root->search(o);

    // no node found, so grow nodes
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
        spdlog::error(
            "BinPack Error: Can't determine correct growth direction of bin");
        throw std::runtime_error(
            "BinPack: Can't determine correct growth direction of bin");
      }

      // re-attempt search
      result = root->search(o);
      // if search failed after growing, something is wrong, throw error
      if (!result) {
        spdlog::error("BinPack error: failed to fit object after resize");
        throw std::runtime_error(
            "Bin Packing error: failed to fit object after growing node");
      }
    }
    spdlog::debug("BinPack: adding object to tree: {0:d}x{1:d} @ {2:d},{3:d}",
                  o->width(), o->length(), result->x, result->y);
    // add object to suitable node
    result->add_object(o);
  }

  // return the dimensions of the bin, which are also the dimensions of the root
  // node
  return std::make_pair(root->width, root->length);
}

void Packer::translate(double offset_x, double offset_y) {
  spdlog::debug("BinPack: translating objects to new location");
  // translate each object to its destination
  root->translate(offset_x, offset_y);
}

} // namespace sse
