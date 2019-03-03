#include <sse/Object.hpp>

/**
 * @brief Object::Object
 * @param s
 */
Object::Object(TopoDS_Shape s){
    this->shape = s;
    // calculate the bounding box
    generate_bounds();

}

/**
 * @brief Object::generate_bounds
 */
void Object::generate_bounds() {
    // FIXME can I just clear the bounding box?
    bounding_box = Bnd_Box{};
    // create bounding box
    BRepBndLib::Add(shape, bounding_box);
}

/**
 * @brief Object::layFlat
 * @param face
 */
void Object::layFlat(const TopoDS_Face &face) {
    // create a face on the XY plane
    auto basePlane  = BRepBuilderAPI_MakeFace(gp_Pln(gp_Pnt(0,0,0),gp_Dir(0,0,1)));
    // measure the angle between the face and the XY plane
    // get the normal of the face
    auto a = GeomLProp_SLProps(BRep_Tool::Surface(face), 0, 0);
    auto normal = a.Normal();



    // rotate
    rotate(0,0,0);
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
void Object::rotate(const long x, const long y, const long z) {
    auto transform = gp_Trsf{};

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

/**
 * @brief Object::getFootprint
 */
TopoDS_Shape Object::getFootprint() {
    Handle(HLRBRep_Algo)a = new HLRBRep_Algo();
    a->Add(shape);
    // FIXME choose the correct vector
    auto p = Prs3d_Projector(false,0, 0 ,0, 0, 0, 0, 0 ,0, 0, 0);
    a->Projector(p.Projector());
    a->Update();
    auto h = HLRBRep_HLRToShape(a);
    TopoDS_Shape proj = h.VCompound();

    return proj;

}
