#include <doctest/doctest.h>

#include "sse/slicer.hpp"

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeHalfSpace.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <gp.hxx>
#include <gp_Pln.hxx>

#include <cmath>
#include <iostream>
#include <memory>

using Objects = std::vector<std::unique_ptr<sse::Object>>;

// precision for comparing values within OCCT
#define SSE_TEST_PRECISION 0.000001

TEST_SUITE("Rearrange Objects") {

  TEST_CASE("Invalid tests") {
    // create list of objects to rearrange
    auto objects = Objects{};
    REQUIRE(objects.empty());

    SUBCASE("Empty object") {
      // add object with zero volume
      auto a = TopoDS_Shape();
      objects.push_back(std::make_unique<sse::Object>(a));
      CHECK_THROWS_AS(sse::rearrange_objects(objects, 500, 500), std::invalid_argument);
    }

    SUBCASE("Infinite volume object") {
      // XY plane
      auto face = BRepBuilderAPI_MakeFace(gp_Pln(gp::Origin(), gp::DZ()));
      // create a halfspace
      auto halfspace = BRepPrimAPI_MakeHalfSpace(face, gp_Pnt(0, 0, -1));
      auto s = halfspace.Shape();
      auto o = std::make_unique<sse::Object>(s);
      objects.push_back(std::move(o));
      // attempt to rearrange object
      CHECK_THROWS_AS(sse::rearrange_objects(objects, 500, 500), std::invalid_argument);
    }

    SUBCASE("Too many objects") {
      // create excessive objects objects
      BRepPrimAPI_MakeBox box{1, 1, 1};
      for (auto i = 0; i <= SSE_MAXIMUM_NUM_OBJECTS; ++i) {
        auto a = box.Shape();
        objects.push_back(std::make_unique<sse::Object>(a));
      }
      CHECK_THROWS_AS(sse::rearrange_objects(objects, 500, 500), std::invalid_argument);
    }

    SUBCASE("Invalid build plate dimensions") {
      BRepPrimAPI_MakeBox b{1, 1, 1};
      auto box = b.Shape();
      objects.push_back(std::make_unique<sse::Object>(box));
      CHECK_THROWS_AS(sse::rearrange_objects(objects, 0, 0), std::invalid_argument);
    }

    SUBCASE("Single object too large for build volume") {
      BRepPrimAPI_MakeBox b{10, 10, 10};
      auto box = b.Shape();
      objects.push_back(std::make_unique<sse::Object>(box));

      CHECK_THROWS_AS(sse::rearrange_objects(objects, 1, 1), std::invalid_argument);
    }

    SUBCASE("Rearranged objects too large for build plate") {
      BRepPrimAPI_MakeBox b{10, 10, 10};
      auto box = b.Shape();
      objects.push_back(std::make_unique<sse::Object>(box));
      objects.push_back(std::make_unique<sse::Object>(box));

      CHECK_THROWS_AS(sse::rearrange_objects(objects, 11, 11), std::runtime_error);
    }

    SUBCASE("Objects vector contains nullptr") {
      objects.push_back(nullptr);

      CHECK_THROWS_AS(sse::rearrange_objects(objects, 100, 100), std::invalid_argument);
    }

  }

  TEST_CASE("Valid Tests") {
    auto objects = Objects{};
    REQUIRE(objects.empty());

    // create a simple box
    BRepPrimAPI_MakeBox box_maker{10, 10, 10};
    auto a = box_maker.Shape();
    REQUIRE(!a.IsNull());

    SUBCASE("Empty argument list") {
      CHECK_NOTHROW(sse::rearrange_objects(objects, 1, 1));
    }

    SUBCASE("One Cube") {
      objects.push_back(std::make_unique<sse::Object>(a));
      CHECK_NOTHROW(sse::rearrange_objects(objects, objects[0]->width(), objects[0]->length()));
      // calculate the correct move location
      gp_Pnt corner{0, 0, 0};
      CHECK(corner.IsEqual(objects[0]->get_bound_box().CornerMin(), SSE_TEST_PRECISION));
    }

  }

  void check_cubes(int num) {
    // create a list of identical objects
    Objects o;
    BRepPrimAPI_MakeBox box_maker{10, 10, 10};
    auto a = box_maker.Shape();
    for(auto i = 0; i < num; ++i) {
        o.push_back(std::make_unique<sse::Object>(a));
    }

    // dimensions of one box
    auto box_width = o[0]->width();
    auto box_length = o[0]->length();
    // the bin dimensions of a list of identical cubes (X elements long)
    // is ceil(sqrt(x)) x round(sqrt(x))
    auto correct_width = ceil(sqrt(num)) * box_width;
    auto correct_length = round(sqrt(num)) * box_length;

    CHECK_NOTHROW(sse::rearrange_objects(o, correct_width, correct_length));

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
      CHECK(o[i]->get_bound_box().CornerMin().IsEqual(correct_location, SSE_TEST_PRECISION));
    }
  }

  TEST_CASE("Implementation Details") {
    auto objects = Objects{};
    REQUIRE(objects.empty());

    // create a simple box
    BRepPrimAPI_MakeBox box_maker{10, 10, 10};
    auto a = box_maker.Shape();
    REQUIRE(!a.IsNull());

    SUBCASE("Should Grow Right") {
      BRepPrimAPI_MakeBox rectangle{10, 20, 10};
      auto r = rectangle.Shape();
      objects.push_back(std::make_unique<sse::Object>(r));
      rectangle = BRepPrimAPI_MakeBox{9, 19, 10};
      r = rectangle.Shape();
      objects.push_back(std::make_unique<sse::Object>(r));

      auto w = objects[0]->width() + objects[1]->width();
      auto l = objects[0]->length();

      CHECK_NOTHROW(sse::rearrange_objects(objects, w, l));

      gp_Pnt corner1{0, 0, 0};
      CHECK(corner1.IsEqual(objects[0]->get_bound_box().CornerMin(), SSE_TEST_PRECISION));

      gp_Pnt corner2{objects[0]->width(), 0, 0};
      CHECK(corner2.IsEqual(objects[1]->get_bound_box().CornerMin(), SSE_TEST_PRECISION));
    }

    SUBCASE("Can Grow Up") {
      // have to exhaust should_grow_* and can_grow_right in order to hit this codepath
      BRepPrimAPI_MakeBox rectangle{20, 10, 10};
      auto r = rectangle.Shape();
      objects.push_back(std::make_unique<sse::Object>(r));
      rectangle = BRepPrimAPI_MakeBox{10, 20, 10};
      r = rectangle.Shape();
      objects.push_back(std::make_unique<sse::Object>(r));

      auto w = objects[0]->width();
      auto l = objects[0]->length() + objects[1]->length();

      CHECK_NOTHROW(sse::rearrange_objects(objects, w, l));

      gp_Pnt corner1{0, 0, 0};
      CHECK(corner1.IsEqual(objects[0]->get_bound_box().CornerMin(), SSE_TEST_PRECISION));

      gp_Pnt corner2{0, objects[0]->length(), 0};
      CHECK(corner2.IsEqual(objects[1]->get_bound_box().CornerMin(), SSE_TEST_PRECISION));
    }

    SUBCASE("List of identical cubes:") {
      for (auto i = 1; i <= 16; ++i) {
        CAPTURE(i);
        check_cubes(i);
      }
    }
  }

}

