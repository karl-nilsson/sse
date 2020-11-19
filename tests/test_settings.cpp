#include <doctest/doctest.h>

#include <sse/Settings.hpp>
#include <iostream>
#include <fstream>


TEST_SUITE("Settings") {

  TEST_CASE("Invalid Ops") {
  sse::Settings &settings{sse::Settings::getInstance()};

    SUBCASE("Bad Parse") {
      // parsing invalid toml file
      CHECK_THROWS_AS(settings.parse("resources/invalid_profile.toml"), toml::syntax_error);
    }

    SUBCASE("File Does Not Exist") {
      CHECK_THROWS(settings.parse(""));
    }

    SUBCASE("Get Invalid Setting") {
      CHECK_THROWS_AS(settings.get_setting<int>("absent_value"), toml::exception);
    }

    SUBCASE("Get Incorrect Setting Type") {
      CHECK_THROWS_AS(settings.get_setting<std::string>("layer_height"), toml::type_error);
    }


  }


  TEST_CASE("Valid Ops") {
    sse::Settings &settings{sse::Settings::getInstance()};

    SUBCASE("Parse") {
      settings.parse("resources/profile.toml");
    }

    SUBCASE("Dump") {
      CHECK_NOTHROW(settings.dump());
    }

    SUBCASE("Save") {
      settings.save();
    }

    SUBCASE("Get Setting") {
      settings.parse("resources/profile.toml");
      CHECK_NOTHROW(settings.get_setting<int>("shells"));
    }

    SUBCASE("Get Setting w/Fallback") {
      settings.parse("resources/profile.toml");
      CHECK_EQ(settings.get_setting_fallback<int>("invalid", 1000), 1000);
    }

    SUBCASE("Get Nested Setting") {
      // settings.get_nested_setting<int>("level1", "level2", "level3");
    }

    SUBCASE("Set Setting") {
      settings.set_setting<int>("test_value", 100);
      CHECK_EQ(settings.get_setting<int>("test_value"), 100);
    }



  }
}
