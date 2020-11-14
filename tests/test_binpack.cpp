#include <doctest/doctest.h>

#include <sse/Packer.hpp>

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeHalfSpace.hxx>

#include <cmath>
#include <iostream>
#include <memory>

using Objects = std::vector<std::shared_ptr<sse::Object>>;

TEST_SUITE("Bin Packer") {

  TEST_CASE("Invalid tests") {
    // create list of objects to pack
    auto objects = Objects{};
    REQUIRE(objects.empty());

    SUBCASE("Empty object") {
      // add object with zero volume
      auto a = TopoDS_Shape();
      objects.push_back(std::make_shared<sse::Object>(a));
      CHECK_THROWS_AS(auto p = sse::Packer(objects), std::runtime_error);
    }

    SUBCASE("Infinite volume object") {
      // XY plane
      auto face = BRepBuilderAPI_MakeFace(gp_Pln(gp::Origin(), gp::DZ()));
      // create a halfspace
      auto halfspace = BRepPrimAPI_MakeHalfSpace(face, gp_Pnt(0, 0, -1));
      auto s = halfspace.Shape();
      auto o = std::make_shared<sse::Object>(s);
      objects.push_back(std::move(o));
      // attempt to pack object
      CHECK_THROWS_AS(auto p = sse::Packer(objects), std::runtime_error);
    }

    SUBCASE("Too many objects") {
      // create excessive objects objects
      BRepPrimAPI_MakeBox box{1, 1, 1};
      // MAXIMUM_OBJECTS is defined in Packer.hpp
      for (auto i = 0; i <= MAXIMUM_OBJECTS; ++i) {
        auto a = box.Shape();
        objects.push_back(std::make_shared<sse::Object>(a));
      }
      CHECK_THROWS_AS(auto p = sse::Packer(objects), std::runtime_error);
    }
  }

  void check_cubes(int num, Objects& l) {
    // create a list of idenital objects
    BRepPrimAPI_MakeBox box_maker{10, 10, 10};
    auto a = box_maker.Shape();
    for(auto i = 0; i < num; ++i) {
        l.push_back(std::make_shared<sse::Object>(a));
    }

    auto p = sse::Packer{l};
    p.pack();

    // the bin dimensions of a list of identical cubes (X elements long)
    // is ceil(sqrt(x)) x round(sqrt(x))

    auto bin_width = ceil(sqrt(num)) * l[0]->width();
    auto bin_height = round(sqrt(num)) * l[0]->width();
    auto result = p.pack();
    // check bin dimensions
    CHECK(result.first == doctest::Approx(bin_width));
    CHECK(result.second == doctest::Approx(bin_height));

    // arrange cubes
    p.arrange(0, 0);
    // check location of each moved cube
    for(std::size_t i = 0; i < l.size(); ++i) {
        double x = 0;
        double y = 0;

        CHECK(l[i]->get_bound_box().CornerMin().X() == doctest::Approx(x));
        CHECK(l[i]->get_bound_box().CornerMin().Y() == doctest::Approx(y));

    }
  }

  TEST_CASE("Valid Tests") {
    // create list of objects to pack
    auto objects = Objects{};
    REQUIRE(objects.empty());

    // create a simple box
    BRepPrimAPI_MakeBox box_maker{10, 10, 10};
    auto a = box_maker.Shape();
    REQUIRE(!a.IsNull());

    SUBCASE("Empty argument list") {
      auto p = sse::Packer(objects);
      auto result = p.pack();
      CHECK(result.first == doctest::Approx(0.0));
      CHECK(result.second == doctest::Approx(0.0));

      CHECK_NOTHROW(p.arrange(0, 0));
    }

    SUBCASE("One Cube") {
      objects.push_back(std::make_shared<sse::Object>(a));
      REQUIRE(objects.size() == 1);
      auto p = sse::Packer(objects);
      auto [x, y] = p.pack();
      CHECK(objects[0]->width() == doctest::Approx(x));
      CHECK(objects[0]->length() == doctest::Approx(y));
      p.arrange(0, 0);
      // calculate the correct move location
      gp_Pnt corner{0, 0, 0};
      // FIXME: currently broken
      auto a = objects[0]->get_bound_box().CornerMin();
      CHECK(corner.IsEqual(objects[0]->get_bound_box().CornerMin(), 0.000001));
    }

    SUBCASE("Double Pack") {
      objects.push_back(std::make_shared<sse::Object>(a));
      REQUIRE(objects.size() == 1);
      auto p = sse::Packer(objects);
      // attempt to pack twice, this should work
      p.pack();
      p.pack();
      p.arrange(0,0);
    }

    SUBCASE("Double Arrange") {
      objects.push_back(std::make_shared<sse::Object>(a));
      auto p = sse::Packer{objects};

      p.pack();
      gp_Pnt corner{0,0,0};
      gp_Pnt corner2{100,100,0};

      p.arrange(0,0);
      CHECK(corner.IsEqual(objects[0]->get_bound_box().CornerMin(), 0.000001));
      p.arrange(100, 100);
      CHECK(corner2.IsEqual(objects[0]->get_bound_box().CornerMin(), 0.000001));
    }

    SUBCASE("List of identical cubes:") {
      for (auto i = 1; i < 16; ++i) {
        CAPTURE(i);
        check_cubes(i, objects);
      }
    }
  }
}
