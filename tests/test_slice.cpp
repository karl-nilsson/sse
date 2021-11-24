#include <doctest/doctest.h>

#include <sse/Slice.hpp>

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>

#include <gp.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Circ.hxx>

#include <random>
#include <vector>
#include <algorithm>


TEST_SUITE("Slice") {
  std::random_device d;
  std::default_random_engine engine(d());
  std::uniform_real_distribution<double> dist(-1000.0, 1000.0);

  double layer_height = 0.2;

  auto s = TopoDS_Shape();

  TEST_CASE("Invalid Ops") {
    auto shape = TopoDS_Shape();

    SUBCASE("Empty Shape") {
      TopoDS_Face f;

      CHECK_THROWS_AS(sse::Slice(nullptr, f, layer_height), std::invalid_argument);
    }
  }

  TEST_CASE("Valid Ops") {
    auto shape = TopoDS_Face();


    SUBCASE("Test random z location") {
      auto random_z = dist(engine);

      auto p = gp_Pnt(0, 0, random_z);
      auto wire = BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(gp_Circ(gp_Ax2(p, gp::DZ()), 1)));
      CHECK_EQ(wire.IsDone(), true);
      const auto face_maker = BRepBuilderAPI_MakeFace(wire.Wire(), true);
      CHECK_EQ(face_maker.IsDone(), true);
      TopoDS_Face f = face_maker.Face();
      const auto slice = sse::Slice(nullptr, f, layer_height);
      CHECK_EQ(slice.z_position(), doctest::Approx(random_z));
    }

    SUBCASE("Test z value: incremental") {
      for(int i = 0; i < 10; ++i) {
        auto p = gp_Pnt(0, 0, i);
        // create a circle with radius 1
        auto wire = BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(gp_Circ(gp_Ax2(p, gp::DZ()), 1)));
        CHECK_EQ(wire.IsDone(), true);
        // create face from circle
        auto face_maker = BRepBuilderAPI_MakeFace(wire.Wire(), true);
        CHECK_EQ(face_maker.IsDone(), true);

        TopoDS_Face f = face_maker.Face();
        auto slice = sse::Slice(nullptr, f, layer_height);

        // check computed Z value to original location
        REQUIRE(slice.z_position() == doctest::Approx(i));
      }
    }

    SUBCASE("Shells") {
      auto offsets = std::vector<double>{1, 2, 3};
      // create a circle with radius 1
      auto wire = BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(gp_Circ(gp_Ax2(gp::Origin(), gp::DZ()), 10)));
      CHECK_EQ(wire.IsDone(), true);
      auto face_maker = BRepBuilderAPI_MakeFace(wire.Wire(), true);
      CHECK_EQ(face_maker.IsDone(), true);

      TopoDS_Face f = face_maker.Face();
      auto slice = sse::Slice(nullptr, f, layer_height);

      CHECK_NOTHROW(slice.generate_shells(1, 1));
      CHECK_NOTHROW(static_cast<void>(slice.gcode(1, 1, 1)));
    }
  }

}
