#include <doctest/doctest.h>

#include <sse/Slice.hpp>

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepBuilderAPI_Transform.hxx>

#include <random>


TEST_SUITE("Slice") {

  TEST_CASE("Invalid Ops") {
    auto shape = TopoDS_Shape();


  }

  TEST_CASE("Valid Ops") {
    auto shape = TopoDS_Shape();

    SUBCASE("Empty Shape") {
      auto s = TopoDS_Shape();
      auto slice = sse::Slice(s);

      REQUIRE_EQ(slice.get_faces().Size(), 0);
    }

    SUBCASE("< Operator") {
      //

      auto b = BRepPrimAPI_MakeBox(10, 10, 10);
      // create a box at (0,0,0)
      auto box1 = b.Shape();
      auto slice1 = sse::Slice(box1);
      // create a second box at (0,0,10)
      auto transform = gp_Trsf();
      transform.SetTranslation(gp_Vec(0,0,1));
      auto box2 = BRepBuilderAPI_Transform(b.Shape(), transform).Shape();
      auto slice2 = sse::Slice(box2);

      CHECK_LT(slice1, slice2);

    }

    SUBCASE("Sort Simple Values") {

      auto slices = std::vector<sse::Slice>();
          // std::vector<std::reference_wrapper<sse::Slice>>();
      auto b = BRepPrimAPI_MakeBox(10, 10, 10);

      for(int i = 10; i >= -10; --i) {
          auto z = b.Shape();
          auto s = sse::Slice(z);
          slices.push_back(s);
          slices.back().translate(0, 0, i * 10);
      }

      std::sort(slices.begin(), slices.end(), [](const std::reference_wrapper<sse::Slice> &lhs, const std::reference_wrapper<sse::Slice> &rhs){
          return lhs.get().get_bound_box().CornerMin().Z() <= rhs.get().get_bound_box().CornerMin().Z();
        });

      // ensure the vector is sorted correctly
      auto prev = slices.front().get_bound_box().CornerMin().Z();
      for(const auto &s: slices) {
          CHECK_LE(prev, s.get_bound_box().CornerMin().Z());
          prev = s.get_bound_box().CornerMin().Z();
      }
    }

    SUBCASE("Sort Random Values") {
      auto slices = std::vector<sse::Slice>();
      auto b = BRepPrimAPI_MakeBox(10, 10, 10);

      std::default_random_engine gen;
      auto r = std::uniform_int_distribution(-100, 100);
      for(int i = 50; i >= 0; --i) {
          auto z = b.Shape();
          slices.push_back(sse::Slice(z));
          slices.back().translate(0, 0, r(gen));
      }
      std::sort(slices.begin(), slices.end());

      // ensure the vector is sorted correctly
      auto prev = slices.front().get_bound_box().CornerMin().Z();
      for(size_t i = 0; i < slices.size(); ++i) {
          CHECK_LE(prev, slices[i].get_bound_box().CornerMin().Z());
          prev = slices[i].get_bound_box().CornerMin().Z();
      }
    }
  }

}
