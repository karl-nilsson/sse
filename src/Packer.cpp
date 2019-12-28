#include <sse/Packer.hpp>

namespace sse {

Footprint::Footprint(Object &o) : object(o) {
  width = object.width();
  length = object.length();
}



/**
 * @brief Packer::Packer
 * @param x
 * @param y
 */
Packer::Packer() : config(Settings::getInstance()) {
  // get build plate dimensions from settings
}

/**
 * @brief Packer::add_object append object to current list of objects
 * @param o
 */
void Packer::add_object(Object &o) { objects.push_back(Footprint(o)); }

/**
 * @brief Packer::add_objects append new objects to current list of objects
 * @param o
 */
void Packer::add_objects(std::vector<Object &> o) {
  for (auto i : o) {
    add_object(i);
  }
}



Node::Node(double x, double y, double l, double w) {}

void Node::add_footprint(Footprint *f) { footprint = f; }

/**
 * @brief Node::insert Recursively traverse the binary tree, searching for a
 * suitable place to fit the object's footprint
 * @param f The footprint of the object to insert
 * @return pointer to a suitable node, nullptr otherwise
 */
Node *Node::insert(Footprint f) {
  // if node is a leaf
  if (this->child[0] == nullptr && this->child[1] == nullptr) {
    // if node is full, return
    if (this->full())
      return nullptr;
    // if too small, return
    if (!this->fits(f))
      return nullptr;
    // otherwise, fits!

    // split node and add children
    this->child[0] = new Node(this->x + f.width, this->y, 100, 100);
    this->child[1] = new Node(this->x, this->y + length, 100, 100);

    return this->child[0]->insert(f);

  } else {
    // try to insert to left child
    auto n = this->child[0]->insert(f);
    // success! return
    if (n != nullptr)
      return n;
    // otherwise, try right child
    return child[1]->insert(f);
  }
}
/**
 * @brief translate Recursively visit every node in the tree, and translate each object to its new position
 *
 */
void Node::translate() {
  // only translate if there's an object inside
  if (this->full()) {
    // calculate translation
    auto move_x = this->x - this->footprint->x;
    auto move_y = this->y - this->footprint->y;
    // perform translation
    this->footprint->object.translate(move_x, move_y, 0);
  }

  // visit children
  if (this->child[0] != nullptr)
    this->child[0]->translate();
  if (this->child[1] != nullptr)
    this->child[1]->translate();
}

/**
 * @brief Packer::pack Pack the objects into the smallest rectangle possible
 */
void Packer::pack() {
  // first, sort the objects, biggest to smallest
  std::sort(objects.begin(), objects.end());
  // create the root node
  auto root = Node(0, 0, objects.front().width, objects.front().length);
  // insert all footprints into the tree
  for (auto o : objects) {
    // if we ran out of space, grow the available space and try again
    if (root.insert(o) == nullptr) {
      // determine whether to grow right or down

      // if it fails a second time, something is wrong
      if (root.insert(o) == nullptr) {
        // raise error
      }
    }
  }
  // check to see if the pack fit within the build plate

  // translate each object to its destination
  root.translate();
}

} // namespace sse
