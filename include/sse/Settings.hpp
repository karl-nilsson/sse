#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include "cpptoml.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

class Settings {

public:
    Settings();
    Settings(fs::path file);
    void export_settings();
private:
};

#endif // SETTINGS_HPP
