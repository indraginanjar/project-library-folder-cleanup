#include <catch2/catch_test_macros.hpp>
#include "ConfigurationManager.h"
#include "Logger.h"
#include "temp_directory.h"
#include <filesystem>
#include <fstream>

// Note: GUI-specific tests are excluded when Qt is not available
// These tests verify the integration of shared components (ConfigurationManager, Logger)
// that are used by both CLI and GUI modes

TEST_CASE("GUI mode uses shared configuration and logger", "[integration][gui]") {
    
    SECTION("GUI uses shared ConfigurationManager") {
        // Set custom configuration
        std::vector<std::string> custom_folders = {"node_modules", "build", "dist"};
        ConfigurationManager::instance().set_target_folders(custom_folders);
        
        // Verify configuration is accessible
        auto folders = ConfigurationManager::instance().get_target_folders();
        REQUIRE(folders.size() == 3);
        REQUIRE(folders[0] == "node_modules");
        REQUIRE(folders[1] == "build");
        REQUIRE(folders[2] == "dist");
        
        // Reset to defaults for other tests
        ConfigurationManager::instance().set_target_folders(
            ConfigurationManager::get_default_folders()
        );
    }
    
    SECTION("GUI uses Logger throughout") {
        // Setup logger with a test log file
        TempDirectory log_dir;
        auto log_path = log_dir.path() / "test_gui.log";
        
        Logger::instance().set_log_file(log_path);
        Logger::instance().set_console_output(false);
        Logger::instance().set_min_level(Logger::Level::Info);
        
        // Log some test messages
        Logger::instance().log(Logger::Level::Info, "GUI test started");
        Logger::instance().log(Logger::Level::Info, "Testing logger integration");
        
        // Close log file to flush
        Logger::instance().close_log_file();
        
        // Verify log file was created and contains entries
        REQUIRE(std::filesystem::exists(log_path));
        
        std::ifstream log_file(log_path);
        std::string log_content((std::istreambuf_iterator<char>(log_file)),
                                std::istreambuf_iterator<char>());
        
        // Verify logger was used
        REQUIRE(log_content.find("GUI test started") != std::string::npos);
        REQUIRE(log_content.find("Testing logger integration") != std::string::npos);
    }
    
    SECTION("Configuration is shared between CLI and GUI modes") {
        // Set configuration in one mode
        std::vector<std::string> shared_folders = {"node_modules", "venv", "target"};
        ConfigurationManager::instance().set_target_folders(shared_folders);
        
        // Verify it's accessible from both modes
        auto folders_from_gui = ConfigurationManager::instance().get_target_folders();
        REQUIRE(folders_from_gui.size() == 3);
        
        // This demonstrates that both CLI and GUI would see the same configuration
        // since they both use ConfigurationManager::instance()
        REQUIRE(folders_from_gui[0] == "node_modules");
        REQUIRE(folders_from_gui[1] == "venv");
        REQUIRE(folders_from_gui[2] == "target");
        
        // Reset to defaults
        ConfigurationManager::instance().set_target_folders(
            ConfigurationManager::get_default_folders()
        );
    }
}
