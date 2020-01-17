#pragma once

#include <algorithm>
#include <exception>
#include <vector>

#include <sse/Object.hpp>

#include <spdlog/spdlog.h>

/**
 * TODO:
 * allow for configurable OFFSET space between footprint rectangles
 * (i.e. expand all rectangles individually, based on brim, if applicable.
 * keep in mind, brim may not expand footprint, i.e. brim for a sphere)
 */
// buffer space 10mm each direction
#define OFFSET 10

namespace sse {

/**
 * @brief Binary tree Node
 * This class describes a binary tree Node that corresponds to a rectangle in the cartesian
 * plane. For the purposes of this class, width is a dimension in the X axis, and length in the Y axis.
 */
class Node {
  using node_ptr = std::shared_ptr<Node>;
public:
  double x, y, width, length;
  Node(double x, double y, double w, double l);
  // check to see if a rectangle will fit in this node
  bool fits(std::shared_ptr<Object> o) {
    return (o->length() + OFFSET < length) && (o->width() + OFFSET < width);
  }
  // check to see if there's an object in this node
  bool full() { return bool(object); }
  // check to see if this node is a leaf node or not
  bool leaf() { return bool(up);}
  Node *search(std::shared_ptr<Object> o);
  void add_object(std::shared_ptr<Object> o);
  void translate(double offset_x, double offset_y);
  void add_children(node_ptr up, node_ptr right);
private:
  node_ptr up, right;
  std::shared_ptr<Object> object;
};

/**
 * @brief Pack objects into the smallest rectangular bin, based on their XY bounding
 * box. Construct a binary tree of Nodes, growing the bin and tree to fit
 * objects. A few heuristics are employed to minimize wasted space, ensuring the resulting bin has close to equal dimensions (i.e. roughly a
 * square).
 *
 */
class Packer {

public:
  Packer(std::vector<std::shared_ptr<Object>> objects);
  std::pair<double, double> pack();
  void translate(double offset_x, double offset_y);
private:
  void grow_up(double w, double l);
  void grow_right(double w, double l);
  std::vector<std::shared_ptr<Object>> objects;
  std::shared_ptr<Node> root;
};

} // namespace sse
