#include <doctest/doctest.h>

#include <sse/Packer.hpp>

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeHalfSpace.hxx>

#include <math.h>
#include <memory>
#include <iostream>

TEST_CASE("Packer parameter sanitization") {
  // create empty vector
  auto objects = std::vector<std::shared_ptr<sse::Object>>();
  // ensure vector is empty
  REQUIRE(objects.size() == 0);

  SUBCASE("testing empty binpacker") {
    // should throw an error
    CHECK_THROWS(auto p = sse::Packer(objects));
  }

  SUBCASE("testing binpacker with empty object") {
    // add object with zero volume
    objects.push_back(std::make_shared<sse::Object>(TopoDS_Shape()));
    CHECK_THROWS_AS(auto p = sse::Packer(objects), std::runtime_error);
  }

  SUBCASE("testing binpacker with infinite volume object") {
    // XY plane
    auto face = BRepBuilderAPI_MakeFace(gp_Pln(gp::Origin(), gp::DZ()));
    // create a halfspace
    auto halfspace = BRepPrimAPI_MakeHalfSpace(face, gp_Pnt(0, 0, -1));
    auto o = std::make_shared<sse::Object>(halfspace.Shape());
    objects.push_back(std::move(o));
    // attempt to pack object
    CHECK_THROWS_AS(auto p = sse::Packer(objects), std::runtime_error);
  }

  SUBCASE("testing binpacker with 1000 objects") {
    // create 1000 empty objects
    for (auto i = 0; i < 1000; ++i) {
      objects.push_back(std::make_shared<sse::Object>(TopoDS_Shape()));
    }
    CHECK_THROWS_AS(auto p = sse::Packer(objects), std::runtime_error);
  }
}


TEST_CASE("Packer cubes test") {
  // create empty vector
  auto objects = std::vector<std::shared_ptr<sse::Object>>();
  // ensure vector is empty
  REQUIRE(objects.size() == 0);
  // make a box with one corner at (0,0,0), with X,Y,Z dimensions of 100
  auto box = BRepPrimAPI_MakeBox(gp::Origin(), 100, 100, 100).Shape();

  SUBCASE("List of squares") {
    for (auto i = 1; i < 16; i++) {
      CAPTURE(i);
      // generate subcase label
      SUBCASE(fmt::format("{i} squares", i).c_str()) {
        // push all objects to vector
        for (auto j = 0; j < i; ++j) {
          objects.push_back(std::make_shared<sse::Object>(box));
        }
        auto p = sse::Packer(objects);

        // the bin dimensions of a list of cubes (X elements long) will always be
        // ceil(sqrt(x)) x floor(sqrt(x))
        auto correct_result = std::make_pair<double, double>(
            ceil(sqrt(i)) * objects.front()->width(),
            floor(sqrt(i)) *  objects.front()->width());
        // make sure the bin is the correct size
        CHECK_EQ(correct_result, p.pack());
      }
    }
  }
}
