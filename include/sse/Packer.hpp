#pragma once

#include <sse/Object.hpp>
#include <sse/Settings.hpp>

#include <Bnd_Box2d.hxx>
#include <algorithm>
#include <vector>

/**
 * TODO:
 * allow for configurable buffer space between footprint rectangles
 * (i.e. expand all rectangles individually, based on brim, if applicable.
 * keep in mind, brim may not expand footprint, i.e. brim for a sphere)
 */

namespace sse {

/**
 * @brief The footprint of 3D models, using a rectangular bounding box
 */
class Footprint {

public:
  Footprint(Object &o);
  // x,y coords, width = x, length = y
  double x, y, length, width;
  // overload operator to allow rough sorting, based on biggest edge length
  // this greatly simplifies the packing algorithm
  bool operator<(const Footprint &rhs) const {
    return std::max(length, width) > std::max(rhs.length, rhs.width);
  }
  Object &object;

private:
};

/**
 * @brief The Node class
 */
class Node {

public:
  Node(double x, double y, double l, double w);
  // check to see if a rectangle will fit in this node
  bool fits(Footprint f) { return f.length < length && f.width < width; }
  bool full() { return this->footprint == nullptr; }
  Node *insert(Footprint f);
  void translate();
  void add_footprint(Footprint *f);
  void grow(double w, double l) {width += w; length += l;}

private:
  double x, y, length, width;
  Node *child[2];
  Footprint *footprint;
};

/**
 * @brief Pack objects into the smallest 2D space, based on their bounding box
 *
 */
class Packer {

public:
  Packer();
  void add_object(Object &o);
  void add_objects(std::vector<Object &> o);
  void pack();

private:
  std::vector<Footprint> objects;
  double max_x, max_y;
  Settings &config;
};

} // namespace sse
