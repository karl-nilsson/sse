#include <doctest/doctest.h>
#include <nanobench.h>

#include "sse/slicer.hpp"

#include <BRepPrimAPI_MakeBox.hxx>
#include <gp.hxx>
#include <gp_Pln.hxx>
#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <Message_PrinterOStream.hxx>

#include <cmath>
#include <iostream>
#include <memory>

using Objects = std::vector<std::unique_ptr<sse::Object>>;

namespace bench = ankerl::nanobench;

// make a bed large enough for any test object
#define SSE_TEST_BED_SIZE 500



TEST_SUITE("Benchmarks") {

  TEST_CASE("Rearrange") {
    sse::setup_logger(spdlog::level::off);
    auto objects = Objects{};
    REQUIRE(objects.empty());

    // create a simple box
    BRepPrimAPI_MakeBox box_maker{1, 1, 1};
    auto a = box_maker.Shape();
    REQUIRE(!a.IsNull());

    SUBCASE("Maximum identical objects") {
      for(int i = 0; i < SSE_MAXIMUM_NUM_OBJECTS; ++i) {
        objects.push_back(std::make_unique<sse::Object>(a));
      }

      bench::Bench().run("Rearrange objects",
                                     [&] { sse::rearrange_objects(objects, SSE_MAXIMUM_NUM_OBJECTS + 1, SSE_MAXIMUM_NUM_OBJECTS + 1); });
    }
  }

  TEST_CASE("Slice (intersection)") {
    sse::setup_logger(spdlog::level::off);

    auto objects = Objects{};
    REQUIRE(objects.empty());

    std::filesystem::path p;
    auto slicer = sse::Slicer{p};
    

    SUBCASE("Tall prism") {
    // tall box
      BRepPrimAPI_MakeBox box_maker{1, 1, 500};
      auto b = box_maker.Shape();
      auto o = sse::Object{b};
      objects.push_back(std::make_unique<sse::Object>(o));
      sse::rearrange_objects(objects, SSE_TEST_BED_SIZE, SSE_TEST_BED_SIZE);
      const auto layer_height = 0.4;

      bench::Bench().run("Slice tall prism (intersection)", [&]{
          bench::doNotOptimizeAway(
          slicer.slice_object(objects.front().get(), layer_height)
          );
          });

    }

    SUBCASE("Complex cross-section") {

    }

  }

  TEST_CASE("Import objects") {
    sse::setup_logger(spdlog::level::off);
    // suppress output of STEPControl_Reader
    Message::DefaultMessenger()->RemovePrinters(STANDARD_TYPE(Message_PrinterOStream));


    SUBCASE("STEP") {

      bench::Bench().run("Import STEP", []{
        bench::doNotOptimizeAway(sse::import("resources/box.step"));
      });
    }

    SUBCASE("IGES") {
      bench::Bench().run("Import IGES", []{
        bench::doNotOptimizeAway(sse::import("resources/box.iges"));
      });
    }

    SUBCASE("BREP") {
      bench::Bench().run("Import BREP", []{
        bench::doNotOptimizeAway(sse::import("resources/box.brep"));
      });
    }

    SUBCASE("STL") {
      bench::Bench().run("Import STL", []{
        bench::doNotOptimizeAway(sse::import("resources/box.stl"));
      });
    }

    SUBCASE("OBJ") {
      bench::Bench().run("Import OBJ", []{
        bench::doNotOptimizeAway(sse::import("resources/box.obj"));
      });
    }

  }
}

#undef SSE_TEST_BED_SIZE
