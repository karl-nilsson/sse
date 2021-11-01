#include <doctest/doctest.h>

#include <sse/Settings.hpp>
#include <iostream>
#include <fstream>
#include <string>


/**
 * TODO: Figure out a better way to deal with loading files
 * currently, cmake copies example files to the build dir.
 * This works, but is fragile, because the unit test program searches
 * for files relative to the current directory. That is, if you run
 * the unit_test program outside of build/tests, it will fail to find
 * the files, and the unit tests will fail
 */

TEST_SUITE("Settings") {

  TEST_CASE("Invalid Ops") {
  sse::Settings &settings{sse::Settings::getInstance()};

    SUBCASE("Bad Parse") {
      // parsing invalid toml file
      // FIXME: on macos, tomll throws an std::exception here, rather than a syntax_error
      CHECK_THROWS(settings.parse("resources/invalid_profile.toml"));
    }

    SUBCASE("File Does Not Exist") {
      CHECK_THROWS(settings.parse(""));
    }

    SUBCASE("Get Invalid Setting") {
      CHECK_THROWS_AS(static_cast<void>(settings.get_setting<int>("absent_value")), toml::exception);
    }

    SUBCASE("Get Incorrect Setting Type") {
      CHECK_THROWS_AS(static_cast<void>(settings.get_setting<std::string>("layer_height")), toml::type_error);
    }


  }


  TEST_CASE("Valid Ops") {
    sse::Settings &settings{sse::Settings::getInstance()};

    SUBCASE("Parse") {
      settings.parse("resources/profile.toml");
    }

    SUBCASE("Dump") {
      CHECK_NOTHROW(static_cast<void>(settings.dump()));
    }

    SUBCASE("Save") {
      settings.save();
    }

    SUBCASE("Get Setting") {
      settings.parse("resources/profile.toml");
      CHECK_NOTHROW(static_cast<void>(settings.get_setting<int>("shells")));
    }

    SUBCASE("Get Setting w/Fallback") {
      settings.parse("resources/profile.toml");
      CHECK_EQ(settings.get_setting_fallback<int>("invalid", 1000), 1000);
    }

    SUBCASE("Get Nested Setting") {
      // settings.get_nested_setting<int>("level1", "level2", "level3");
    }



  }
}
