#include <catch2/catch_test_macros.hpp>
#include "ApplicationController.h"
#include "temp_directory.h"
#include <filesystem>

TEST_CASE("ApplicationController orchestrates scan and delete workflow", "[integration][controller]") {
    // Create a temporary directory structure
    TempDirectory temp_dir;
    auto base_path = temp_dir.path();
    
    // Create some test directories
    std::filesystem::create_directories(base_path / "project1" / "node_modules");
    std::filesystem::create_directories(base_path / "project2" / "venv");
    std::filesystem::create_directories(base_path / "project3" / "normal_folder");
    
    // Create some files to make the folders non-empty
    std::ofstream(base_path / "project1" / "node_modules" / "test.txt") << "test";
    std::ofstream(base_path / "project2" / "venv" / "test.txt") << "test";
    std::ofstream(base_path / "project3" / "normal_folder" / "test.txt") << "test";
    
    ApplicationController controller;
    
    SECTION("Scan operation validates base directory") {
        ApplicationController::OperationConfig config;
        config.base_directory = "/"; // System directory
        config.target_folders = {"node_modules", "venv"};
        config.deletion_mode = FolderDeleter::Mode::DryRun;
        config.require_confirmation = false;
        
        bool result = controller.execute_scan(config, nullptr);
        REQUIRE_FALSE(result); // Should fail for system directory
    }
    
    SECTION("Scan operation finds target folders") {
        ApplicationController::OperationConfig config;
        config.base_directory = base_path;
        config.target_folders = {"node_modules", "venv"};
        config.deletion_mode = FolderDeleter::Mode::DryRun;
        config.require_confirmation = false;
        
        bool result = controller.execute_scan(config, nullptr);
        REQUIRE(result);
        
        const auto& scan_result = controller.get_scan_result();
        REQUIRE(scan_result.found_folders.size() == 2);
    }
    
    SECTION("Delete operation requires prior scan") {
        ApplicationController controller2;
        bool result = controller2.execute_deletion(nullptr);
        REQUIRE_FALSE(result); // Should fail without prior scan
    }
    
    SECTION("Complete scan-then-delete workflow in dry-run mode") {
        ApplicationController::OperationConfig config;
        config.base_directory = base_path;
        config.target_folders = {"node_modules", "venv"};
        config.deletion_mode = FolderDeleter::Mode::DryRun;
        config.require_confirmation = false;
        
        // Execute scan
        bool scan_result = controller.execute_scan(config, nullptr);
        REQUIRE(scan_result);
        REQUIRE(controller.get_scan_result().found_folders.size() == 2);
        
        // Execute deletion in dry-run mode
        bool delete_result = controller.execute_deletion(nullptr);
        REQUIRE(delete_result);
        
        // Verify folders still exist (dry-run mode)
        REQUIRE(std::filesystem::exists(base_path / "project1" / "node_modules"));
        REQUIRE(std::filesystem::exists(base_path / "project2" / "venv"));
    }
    
    SECTION("Complete scan-then-delete workflow in actual mode") {
        ApplicationController::OperationConfig config;
        config.base_directory = base_path;
        config.target_folders = {"node_modules", "venv"};
        config.deletion_mode = FolderDeleter::Mode::ActualDelete;
        config.require_confirmation = false;
        
        // Execute scan
        bool scan_result = controller.execute_scan(config, nullptr);
        REQUIRE(scan_result);
        REQUIRE(controller.get_scan_result().found_folders.size() == 2);
        
        // Execute deletion in actual mode
        bool delete_result = controller.execute_deletion(nullptr);
        REQUIRE(delete_result);
        
        const auto& deletion_result = controller.get_deletion_result();
        REQUIRE(deletion_result.folders_deleted == 2);
        
        // Verify folders are deleted
        REQUIRE_FALSE(std::filesystem::exists(base_path / "project1" / "node_modules"));
        REQUIRE_FALSE(std::filesystem::exists(base_path / "project2" / "venv"));
        
        // Verify normal folder still exists
        REQUIRE(std::filesystem::exists(base_path / "project3" / "normal_folder"));
    }
    
    SECTION("Cancellation support") {
        ApplicationController::OperationConfig config;
        config.base_directory = base_path;
        config.target_folders = {"node_modules", "venv"};
        config.deletion_mode = FolderDeleter::Mode::DryRun;
        config.require_confirmation = false;
        
        // This just verifies cancel_operation doesn't crash
        controller.cancel_operation();
        
        // Execute scan
        controller.execute_scan(config, nullptr);
        
        // Cancel should work after scan too
        controller.cancel_operation();
    }
}
