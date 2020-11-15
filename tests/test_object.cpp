#include <doctest/doctest.h>

#include <sse/Object.hpp>


#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>

TEST_SUITE("Object") {
  BRepPrimAPI_MakeBox box_maker{10, 10, 10};

  TEST_CASE("Empty Shape") {
    auto shape = TopoDS_Shape();
    auto o = sse::Object(shape);

    CHECK(o.width() == doctest::Approx(0.0));
    CHECK(o.length() == doctest::Approx(0.0));
    CHECK(o.height() == doctest::Approx(0.0));

    CHECK(o.get_volume() == doctest::Approx(0.0));
  }

  TEST_CASE("Basic object") {
    auto box = box_maker.Shape();
    auto o = sse::Object(box);

    CHECK(o.width() == doctest::Approx(10.0));
    CHECK(o.length() == doctest::Approx(10.0));
    CHECK(o.height() == doctest::Approx(10.0));
    CHECK(o.get_volume() == doctest::Approx(1000.0));
  }

  TEST_CASE("") {

  }
}
