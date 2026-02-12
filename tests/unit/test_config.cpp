#include <catch2/catch_test_macros.hpp>
#include "ConfigurationManager.h"
#include <algorithm>

TEST_CASE("ConfigurationManager has default folders", "[config]") {
    auto defaults = ConfigurationManager::get_default_folders();
    
    REQUIRE_FALSE(defaults.empty());
    REQUIRE(std::find(defaults.begin(), defaults.end(), "node_modules") != defaults.end());
    REQUIRE(std::find(defaults.begin(), defaults.end(), "venv") != defaults.end());
    REQUIRE(std::find(defaults.begin(), defaults.end(), ".venv") != defaults.end());
    REQUIRE(std::find(defaults.begin(), defaults.end(), "target") != defaults.end());
}

TEST_CASE("ConfigurationManager validates folder names", "[config]") {
    ConfigurationManager& config = ConfigurationManager::instance();
    
    // Valid names should be accepted
    config.set_target_folders({"valid_folder", "another-folder"});
    auto folders = config.get_target_folders();
    REQUIRE(folders.size() == 2);
    
    // Invalid names with path separators should be rejected
    config.set_target_folders({"invalid/folder", "another\\folder"});
    folders = config.get_target_folders();
    REQUIRE(folders.empty());
}
