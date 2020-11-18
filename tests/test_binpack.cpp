#include <doctest/doctest.h>

#include <sse/Packer.hpp>

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeHalfSpace.hxx>

#include <cmath>
#include <iostream>
#include <memory>

using Objects = std::vector<std::shared_ptr<sse::Object>>;

// precision for comparing values within OCCT
#define TEST_PRECISION 0.000001

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

  void check_cubes(int num) {
    // create a list of identical objects
    Objects o;
    BRepPrimAPI_MakeBox box_maker{10, 10, 10};
    auto a = box_maker.Shape();
    for(auto i = 0; i < num; ++i) {
        o.push_back(std::make_shared<sse::Object>(a));
    }

    auto p = sse::Packer{o};

    // dimensions of one box
    auto box_width = o[0]->width();
    auto box_length = o[0]->length();
    // the bin dimensions of a list of identical cubes (X elements long)
    // is ceil(sqrt(x)) x round(sqrt(x))
    auto correct_width = ceil(sqrt(num)) * box_width;
    auto correct_length = round(sqrt(num)) * box_length;
    auto [width, length] = p.pack();
    // check bin dimensions
    CHECK(width == doctest::Approx(correct_width));
    CHECK(length == doctest::Approx(correct_length));
    // arrange cubes
    p.arrange(0, 0);
    // check location of each moved cube
    auto x = 0;
    auto y = 0;

    for (std::size_t i = 0; i < o.size(); ++i) {
      // square root of the previous perfect square
      auto f = floor(sqrt(i));
      // previous perfect square
      auto prev_square = pow(f, 2);
      // transition from column to row
      auto split_point = prev_square + f;

      if (i < split_point) {
        x = f;
        y = i - prev_square;
      } else {
        y = f;
        x = i - split_point;
      }

      gp_Pnt correct_location{x * box_width, y * box_length, 0};
      // check location of each cube
      CHECK(o[i]->get_bound_box().CornerMin().IsEqual(correct_location, TEST_PRECISION));
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
      auto [width, length] = p.pack();
      CHECK(objects[0]->width() == doctest::Approx(width));
      CHECK(objects[0]->length() == doctest::Approx(length));
      p.arrange(0, 0);
      // calculate the correct move location
      gp_Pnt corner{0, 0, 0};
      // FIXME: currently broken
      CHECK(corner.IsEqual(objects[0]->get_bound_box().CornerMin(), TEST_PRECISION));
    }

    SUBCASE("Should Grow Right") {
      // two identical rectangles, test packer should_grow_right
      BRepPrimAPI_MakeBox rectangle{10, 20, 10};
      auto r = rectangle.Shape();
      objects.push_back(std::make_shared<sse::Object>(r));
      rectangle = BRepPrimAPI_MakeBox{9, 19, 10};
      r = rectangle.Shape();
      objects.push_back(std::make_shared<sse::Object>(r));

      auto p = sse::Packer(objects);
      auto [width, length] = p.pack();
      CHECK(objects[0]->width() + objects[1]->width() == doctest::Approx(width));
      CHECK(objects[0]->length() == doctest::Approx(length));

      p.arrange(0, 0);
      gp_Pnt corner1{0, 0, 0};
      CHECK(corner1.IsEqual(objects[0]->get_bound_box().CornerMin(), TEST_PRECISION));

      gp_Pnt corner2{objects[0]->width(), 0, 0};
      CHECK(corner2.IsEqual(objects[1]->get_bound_box().CornerMin(), TEST_PRECISION));
    }

    SUBCASE("Can Grow Up") {
      // two identical rectangles, test packer should_grow_right
      BRepPrimAPI_MakeBox rectangle{20, 10, 10};
      auto r = rectangle.Shape();
      objects.push_back(std::make_shared<sse::Object>(r));
      rectangle = BRepPrimAPI_MakeBox{10, 20, 10};
      r = rectangle.Shape();
      objects.push_back(std::make_shared<sse::Object>(r));

      auto p = sse::Packer(objects);
      auto [width, length] = p.pack();
    }

    SUBCASE("Double Pack") {
      objects.push_back(std::make_shared<sse::Object>(a));
      REQUIRE(objects.size() == 1);
      auto p = sse::Packer(objects);
      // attempt to pack twice, this should work
      auto [w1, l1] = p.pack();
      auto [w2, l2] = p.pack();

      CHECK(w1 == doctest::Approx(w2));
      CHECK(l1 == doctest::Approx(l2));
      p.arrange(0,0);
    }

    SUBCASE("Double Arrange") {
      objects.push_back(std::make_shared<sse::Object>(a));
      auto p = sse::Packer{objects};

      p.pack();
      gp_Pnt corner{0,0,0};
      gp_Pnt corner2{100,100,0};

      p.arrange(0,0);
      CHECK(corner.IsEqual(objects[0]->get_bound_box().CornerMin(), TEST_PRECISION));
      p.arrange(100, 100);
      CHECK(corner2.IsEqual(objects[0]->get_bound_box().CornerMin(), TEST_PRECISION));
    }

    SUBCASE("List of identical cubes:") {
      for (auto i = 1; i <= 16; ++i) {
        CAPTURE(i);
        check_cubes(i);
      }
    }
  }
}
