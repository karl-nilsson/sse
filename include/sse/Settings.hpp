#pragma once

#include <filesystem>
#include <iostream>
#include <vector>
#include <map>

// #include <toml.hpp>

#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

/**
 * @brief The Settings class
 * Singleton
 */
class Settings {

public:
    void read_file(fs::path file);

    static Settings& getInstance() {
      static Settings instance;
      return instance;
    }

    Settings(Settings const&) = delete;
    void operator=(Settings const&) = delete;
private:
    Settings() {}
};

