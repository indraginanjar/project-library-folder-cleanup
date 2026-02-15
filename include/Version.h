#pragma once

#include <string>

namespace Version {
    constexpr const char* VERSION = "0.0.1";
    constexpr const char* APP_NAME = "Library Folder Cleanup";
    
    inline std::string get_version_string() {
        return std::string(APP_NAME) + " v" + VERSION;
    }
}
