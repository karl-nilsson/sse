#include <doctest/doctest.h>

#include <sse/Packer.hpp>

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeHalfSpace.hxx>

#include <cmath>
#include <memory>
#include <iostream>


TEST_CASE("Bin Packer") {
  // create empty vector
  auto objects = std::vector<std::shared_ptr<sse::Object>>();
  // ensure vector is empty
  REQUIRE(objects.size() == 0);


  SUBCASE("Empty argument list") {
    // should throw an error
    CHECK_THROWS_AS(sse::Packer p{objects}, std::runtime_error);
  }

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

  SUBCASE("Valid Tests") {
    // create a simple box
    BRepPrimAPI_MakeBox box_maker{10, 10, 10};
    auto a = box_maker.Shape();

    SUBCASE("One Cube") {
      objects.push_back(std::make_shared<sse::Object>(a));
      REQUIRE(objects.size() == 1);
      auto p = sse::Packer(objects);
      auto [x,y] = p.pack();
      CHECK_EQ(x, objects[0]->width());
      CHECK_EQ(y, objects[0]->length());
      p.arrange(0,0);
      // calculate the correct move location
      gp_Pnt corner{0,0,0};
      // FIXME: currently broken
      // CHECK(corner.IsEqual(objects[0]->get_bound_box().CornerMin(), 0.0001));
    }

    SUBCASE("Double Pack") {
      objects.push_back(std::make_shared<sse::Object>(a));
      REQUIRE(objects.size() == 1);
      auto p = sse::Packer(objects);
      // attempt to pack twice, this should work
      p.pack();
      p.pack();
    }

    SUBCASE("List of cubes:") {
      for (auto i = 1; i < 16; ++i) {
        CAPTURE(i);
        SUBCASE("") {
          // push all objects to vector
          for (auto j = 0; j < i; ++j) {
            objects.push_back(std::make_shared<sse::Object>(a));
          }
          auto p = sse::Packer(objects);

          // the bin dimensions of a list of cubes (X elements long) will always be
          // ceil(sqrt(x)) x floor(sqrt(x))
          auto correct_result = std::make_pair<double, double>(
                ceil(sqrt(i)) * objects.front()->width(),
                floor(sqrt(i)) *  objects.front()->width());
          // make sure the bin is the correct size
          CHECK_EQ(correct_result, p.pack());
          // arrange cubes
          p.arrange(0,0);
          // check location of each moved cube
        }
      }
    }
  }
}

