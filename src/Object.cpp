#include <sse/Object.hpp>

/**
 * @brief Object::Object
 * @param s
 */
Object::Object(TopoDS_Shape s){
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
    // create a face of the XY plane
    auto basePlane  = BRepBuilderAPI_MakeFace(gp_Pln(gp_Pnt(0,0,0),gp_Dir(0,0,1)));
    const auto base_normal = gp_Dir(0, 0, 1);
    // get the u,v min/max of selected surface
    Standard_Real umin, umax, vmin, vmax;
    BRepTools::UVBounds(face, umin, umax, vmin, vmax);
    // get the normal of the selected face
    // TODO: select the correct parameters
    auto normal = GeomLProp_SLProps(BRep_Tool::Surface(face), umin, vmin, 1, 0.1).Normal();
    // if the face was reversed, reverse the normal
    // TODO: this maybe unnecessary
    if(face.Orientation() == TopAbs_REVERSED) {
        normal.Reverse();
    }
    // the axis of rotation is the cross product of the
    try {
        auto axis = normal.Crossed(base_normal);
        // the rotation angle is the dot product of the two vectors
        auto angle = normal.Dot(base_normal);
        // rotate
        rotate(axis, angle);
    } catch(Standard_ConstructionError e) {
        // planes were parallel, so no need to rotate
    }

    // measure distance between plane and bedplate (z=0)

    // move
    translate(0,0,0);
}

/**
 * @brief Object::rotate
 * @param x
 * @param y
 * @param z
 */
void Object::rotate(const gp_Ax1 axis, const double angle) {
    auto transform = gp_Trsf();
    transform.SetRotation(axis, angle);

}

/**
 * @brief Object::translate
 * @param x
 * @param y
 * @param z
 */
void Object::translate(const long x, const long y, const long z) {
    auto transform = gp_Trsf();
}

void Object::scale(const int x, const int y, const int z) {
    auto scale = gp_Trsf();
    scale.SetForm(gp_TrsfForm());
}
