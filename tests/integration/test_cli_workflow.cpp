#include <catch2/catch_test_macros.hpp>
#include "CLIApplication.h"
#include "ConfigurationManager.h"
#include "Logger.h"
#include "temp_directory.h"
#include <filesystem>
#include <fstream>
#include <sstream>

TEST_CASE("CLI mode uses ApplicationController and shared configuration", "[integration][cli]") {
    // Create a temporary directory structure
    TempDirectory temp_dir;
    auto base_path = temp_dir.path();
    
    // Create test directories
    std::filesystem::create_directories(base_path / "project1" / "node_modules");
    std::filesystem::create_directories(base_path / "project2" / "venv");
    
    // Create files to make folders non-empty
    std::ofstream(base_path / "project1" / "node_modules" / "test.txt") << "test";
    std::ofstream(base_path / "project2" / "venv" / "test.txt") << "test";
    
    SECTION("CLI uses shared ConfigurationManager") {
        // Set custom configuration
        std::vector<std::string> custom_folders = {"node_modules", "custom_folder"};
        ConfigurationManager::instance().set_target_folders(custom_folders);
        
        // Verify configuration is accessible
        auto folders = ConfigurationManager::instance().get_target_folders();
        REQUIRE(folders.size() == 2);
        REQUIRE(folders[0] == "node_modules");
        REQUIRE(folders[1] == "custom_folder");
        
        // Reset to defaults for other tests
        ConfigurationManager::instance().set_target_folders(
            ConfigurationManager::get_default_folders()
        );
    }
    
    SECTION("CLI uses Logger throughout") {
        // Setup logger with a test log file
        TempDirectory log_dir;
        auto log_path = log_dir.path() / "test_cli.log";
        
        Logger::instance().set_log_file(log_path);
        Logger::instance().set_console_output(false);
        Logger::instance().set_min_level(Logger::Level::Info);
        
        // Create CLI application with dry-run and force flags
        std::string base_path_str = base_path.string();
        const char* argv[] = {
            "cleanup_cli",
            "--dry-run",
            "--force",
            base_path_str.c_str()
        };
        
        CLIApplication app(4, const_cast<char**>(argv));
        int exit_code = app.run();
        
        // Verify successful execution
        REQUIRE(exit_code == 0);
        
        // Close log file to flush
        Logger::instance().close_log_file();
        
        // Verify log file was created and contains entries
        REQUIRE(std::filesystem::exists(log_path));
        
        std::ifstream log_file(log_path);
        std::string log_content((std::istreambuf_iterator<char>(log_file)),
                                std::istreambuf_iterator<char>());
        
        // Verify logger was used
        REQUIRE(log_content.find("Application started in CLI mode") != std::string::npos);
        REQUIRE(log_content.find("Starting scan") != std::string::npos);
    }
    
    SECTION("CLI mode selection works correctly") {
        // Test with help flag
        const char* argv_help[] = {"cleanup_cli", "--help"};
        CLIApplication app_help(2, const_cast<char**>(argv_help));
        int exit_code = app_help.run();
        REQUIRE(exit_code == 0);
        
        // Test with dry-run flag
        std::string base_path_str = base_path.string();
        const char* argv_dry[] = {
            "cleanup_cli",
            "--dry-run",
            "--force",
            base_path_str.c_str()
        };
        CLIApplication app_dry(4, const_cast<char**>(argv_dry));
        exit_code = app_dry.run();
        REQUIRE(exit_code == 0);
        
        // Verify folders still exist (dry-run mode)
        REQUIRE(std::filesystem::exists(base_path / "project1" / "node_modules"));
        REQUIRE(std::filesystem::exists(base_path / "project2" / "venv"));
    }
}
