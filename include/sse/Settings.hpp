#pragma once

#include <toml.hpp>

#include <filesystem>
#include <iostream>
#include <vector>
#include <map>

#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

class Settings {

public:
    Settings();
    Settings(fs::path file);
    void export_settings();
private:
};

