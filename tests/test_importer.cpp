#include <doctest/doctest.h>

#include "sse/slicer.hpp"


TEST_SUITE("Importer") {
    TEST_CASE("Invalid") {
        SUBCASE("Empty filename") {
            CHECK_THROWS_AS(static_cast<void>(sse::import("")), std::invalid_argument);
        }

        SUBCASE("No file extension") {
            CHECK_THROWS_AS(static_cast<void>(sse::import("filename")), std::invalid_argument);
        }

        SUBCASE("Invalid filename extension") {
            CHECK_THROWS_AS(static_cast<void>(sse::import("profile.toml")), std::invalid_argument);
        }

        SUBCASE("File does not exist") {
            CHECK_THROWS_AS(static_cast<void>(sse::import("file_does_not_exist.step")), std::invalid_argument);
        }

        SUBCASE("Invalid files") {
            CHECK_THROWS_AS(static_cast<void>(sse::import("resources/invalid_box.step")), std::runtime_error);
            CHECK_THROWS_AS(static_cast<void>(sse::import("resources/invalid_box.iges")), std::runtime_error);
            CHECK_THROWS_AS(static_cast<void>(sse::import("resources/invalid_box.brep")), std::runtime_error);
        }
    }

    TEST_CASE("Valid") {
        SUBCASE("STEP file") {
            CHECK_NOTHROW(static_cast<void>(sse::import("resources/box.step")));
        }

        SUBCASE("STEPZ file") {
            CHECK_NOTHROW(static_cast<void>(sse::import("resources/box.stpZ")));
        }

        SUBCASE("IGES file") {
            CHECK_NOTHROW(static_cast<void>(sse::import("resources/box.iges")));
        }

        SUBCASE("BRep file") {
            CHECK_NOTHROW(static_cast<void>(sse::import("resources/box.brep")));
        }

        SUBCASE("STL file") {
            CHECK_NOTHROW(static_cast<void>(sse::import("resources/box.stl")));
        }

        SUBCASE("OBJ file") {
            CHECK_NOTHROW(static_cast<void>(sse::import("resources/box.obj")));
        }

    }
}


