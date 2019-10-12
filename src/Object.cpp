#include <sse/Object.hpp>

/**
 * @brief Object::Object
 * @param s
 */
Object::Object(TopoDS_Shape s) {
  this->shape = s;
  // calculate the bounding box
  this->bounding_box = Bnd_Box();
  this->footprint = Bnd_Box2d();
  this->generate_bounds();
}

/**
 * @brief Object::generate_bounds
 */
void Object::generate_bounds() {
  // clear bounding box
  this->bounding_box.SetVoid();
  // create bounding box
  BRepBndLib::Add(this->shape, this->bounding_box);
  // clear footprint
  this->footprint.SetVoid();
  // TODO: simplify
  Standard_Real xmin, ymin, xmax, ymax, tmp;
  this->bounding_box.Get(xmin, ymin, tmp, xmax, ymax, tmp);
  this->footprint.Add(gp_Pnt2d(xmin, ymin));
  this->footprint.Add(gp_Pnt2d(xmax, ymax));
}

/**
 * @brief Object::layFlat
 * @param face
 */
void Object::layFlat(const TopoDS_Face &face) {
  // get the u,v bounds of selected surface
  Standard_Real umin, umax, vmin, vmax;
  BRepTools::UVBounds(face, umin, umax, vmin, vmax);
  // get the normal of the selected face at (u,v)
  // TODO: select the correct parameters (u,v), currently chooses the "average"
  // parameters
  auto props = GeomLProp_SLProps(BRep_Tool::Surface(face), (umin + umax) / 2,
                                 (vmin + vmax) / 2, 1, 0.1);
  auto normal = props.Normal();
  // get the coordinate
  auto point = props.Value();
  // if the face was reversed, reverse the normal
  // TODO: this may be unnecessary
  if (face.Orientation() == TopAbs_REVERSED) {
    normal.Reverse();
  }

  // if normals aren't already equal, rotate until they are
  if (!normal.IsEqual(gp::DZ(), 0.0001)) {
    // the axis of rotation is the cross product of the two vectors
    auto axis = normal.Crossed(gp::DZ());
    // TODO: verify angle calc correctness
    // TODO: don't use origin
    rotate(gp_Ax1(gp::Origin(), axis), normal.Angle(gp::DZ()));

    // if rotation happened, recalculate coordinate
    // TODO: make more elegant
    point = GeomLProp_SLProps(BRep_Tool::Surface(face), (umin + umax) / 2,
                              (vmin + vmax) / 2, 1, 0.1)
                .Value();
  }

  // move the shape so that the face is touching the XY plane
  translate(0, 0, -1 * point.Z());
}

void Object::mirror() {
  auto transform = gp_Trsf();
  // TODO: choose mirror point other that origin
  transform.SetMirror(gp::Origin());
  auto s = BRepBuilderAPI_Transform(this->shape, transform);
  s.Shape();
}

/**
 * @brief Object::rotate
 * @param axis axis of rotation
 * @param angle angle of rotation, in degrees
 */
void Object::rotate(const gp_Ax1 axis, const double angle) {
  auto transform = gp_Trsf();
  transform.SetRotation(axis, angle * M_PI / 180);
  try {
    auto s = BRepBuilderAPI_Transform(this->shape, transform).Shape();
    this->shape = s;
  } catch (StdFail_NotDone e) {
  }
}

// TODO: this may be unecessary, or maybe DRY
void Object::rotateX(const double angle) { this->rotate(gp::OX(), angle); }

void Object::rotateY(const double angle) { this->rotate(gp::OY(), angle); }

void Object::rotateZ(const double angle) { this->rotate(gp::OZ(), angle); }

/**
 * @brief Object::translate
 * @param x
 * @param y
 * @param z
 */
void Object::translate(const double x, const double y, const double z) {
  auto transform = gp_Trsf();
  transform.SetTranslation(gp_Vec(x, y, z));
  try {
    auto s = BRepBuilderAPI_Transform(this->shape, transform).Shape();
    this->shape = s;
  } catch (StdFail_NotDone e) {
  }
}

/**
 * @brief Object::scale
 * @param x
 * @param y
 * @param z
 */
void Object::scale(const double x, const double y, const double z) {
  // TODO: get working
  auto v = gp_Vec(x, y, z);
  auto scale = gp_Trsf();
  scale.SetScale(gp::Origin(), x);
  auto s = BRepBuilderAPI_Transform(this->shape, scale);
  s.Shape();
}
