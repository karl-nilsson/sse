#include <sse/Object.hpp>


namespace sse {

/**
 * @brief Object::Object
 * @param s
 */
Object::Object(TopoDS_Shape s) {
  spdlog::info("Initializing object with shape");
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
  spdlog::info("generating bounding box");
  // clear bounding box
  this->bounding_box.SetVoid();
  // create bounding box
  BRepBndLib::Add(this->shape, this->bounding_box);

  spdlog::info("generating footprint");
  // clear footprint
  this->footprint.SetVoid();
  // TODO: simplify
  Standard_Real xmin, ymin, xmax, ymax, tmp;
  this->bounding_box.Get(xmin, ymin, tmp, xmax, ymax, tmp);
  this->footprint.Add(gp_Pnt2d(xmin, ymin));
  this->footprint.Add(gp_Pnt2d(xmax, ymax));
}

/**
 * @brief Object::layFlat rotate and translate object so that one face is flat on the buildplate
 * @param face
 */
void Object::lay_flat(const TopoDS_Face &face) {
  spdlog::info("laying object flat");
  // get the u,v bounds of selected surface
  Standard_Real umin, umax, vmin, vmax;
  BRepTools::UVBounds(face, umin, umax, vmin, vmax);
  // get the normal of the selected face at (u,v)
  // TODO: select the correct parameters (u,v)
  // currently chooses the "average" parameters
  auto props = GeomLProp_SLProps(BRep_Tool::Surface(face), (umin + umax) / 2,
                                 (vmin + vmax) / 2, 1, 1e-6);
  // normal undefined, error
  // TODO: raise error?
  if(! props.IsNormalDefined()) {
      return;
  }

  auto normal = props.Normal();
  spdlog::debug("Face normal: ");
  // get the coordinate of the face at (u,v)
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
    // use midpoint of bounding box as the center of rotation
    auto min = bounding_box.CornerMin();
    auto max = bounding_box.CornerMax();
    spdlog::debug("Axis: ");
    spdlog::debug("Angle: ");
    // TODO: verify center point is correct/good idea
    // TODO: verify angle calc correctness
    rotate(gp_Ax1(gp_Pnt((min.X() + max.X()) / 2, (min.Y() + max.Y()) / 2,
                         (min.Z() + max.Z()) / 2),
                  axis),
           normal.Angle(gp::DZ()));

    // if rotation happened, recalculate coordinate
    // TODO: make more elegant
    point = GeomLProp_SLProps(BRep_Tool::Surface(face), (umin + umax) / 2,
                              (vmin + vmax) / 2, 1, 1e-6)
                .Value();
  }

  // move the shape so that the face is touching the XY plane
  translate(0, 0, -1 * point.Z());
}

/**
 * @brief Object::center_point
 * @return
 */
const gp_Pnt Object::center_point() {
  auto min = bounding_box.CornerMin();
  auto max = bounding_box.CornerMax();
  return gp_Pnt((min.X() + max.X()) / 2, (min.Y() + max.Y()) /2, (min.Z() + max.Z())/2);
}

/**
 * @brief Object::mirror
 * @param mirror_plane
 * TODO: double-check result
 */
void Object::mirror(gp_Ax2 mirror_plane) {
  spdlog::debug("Mirror: ");
  auto transform = gp_Trsf();
  transform.SetMirror(mirror_plane);
  auto s = BRepBuilderAPI_Transform(this->shape, transform);
  this->shape = s.Shape();
}

/**
 * @brief Object::rotate
 * @param axis axis of rotation
 * @param angle angle of rotation, in degrees
 */
void Object::rotate(const gp_Ax1 axis, const double angle) {
  spdlog::debug("Rotating object");
  auto transform = gp_Trsf();
  transform.SetRotation(axis, angle * M_PI / 180);
  try {
    auto s = BRepBuilderAPI_Transform(this->shape, transform).Shape();
    this->shape = s;
  } catch (StdFail_NotDone& e) {
    e.Print(std::cerr);
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
  spdlog::debug("Translating: ");
  auto transform = gp_Trsf();
  transform.SetTranslation(gp_Vec(x, y, z));
  try {
    auto s = BRepBuilderAPI_Transform(this->shape, transform).Shape();
    this->shape = s;
  } catch (StdFail_NotDone& e) {
    e.Print(std::cerr);
  }
}

/**
 * @brief Object::scale
 * @param x
 * @param y
 * @param z
 */
void Object::scale(const double x, const double y, const double z) {
  spdlog::debug("Scaling: ");

  // TODO: get working
  auto v = gp_Vec(x, y, z);
  auto scale = gp_Trsf();
  scale.SetScale(center_point(), x);
  auto s = BRepBuilderAPI_Transform(this->shape, scale);
  s.Shape();
}

const double Object::get_volume() {
  GProp_GProps volume;
  BRepGProp::VolumeProperties(this->shape, volume);
  return volume.Mass();
}

} // namespace sse
