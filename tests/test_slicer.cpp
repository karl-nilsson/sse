#include <doctest/doctest.h>

#include "sse/slicer.hpp"

TEST_SUITE("Slicer") {

    TEST_CASE("Valid Ops") {
        auto slicer = sse::Slicer{};

        SUBCASE("Slices Number") {
            const auto height = 10.0
            auto shape = BrepPrimAPI_MakeBox{1, 1, height};
            auto object = sse::Object{shape};
            const auto layer_height = 0.1;

            auto slices = sse::slice_object(object, layer_height);
            ASSERT_EQ(slices.size(), (int)(height / layer_height));
            
        }

    }

    TEST_CASE("Invalid Ops") {

    }
}