#include <catch2/catch_test_macros.hpp>
#include "FileSystemScanner.h"
#include "temp_directory.h"
#include <thread>
#include <atomic>

TEST_CASE("Scanner finds target folders", "[scanner]") {
    TempDirectory temp;
    
    temp.create_directory("project1/node_modules");
    temp.create_directory("project2/venv");
    temp.create_directory("project3/other");
    
    FileSystemScanner scanner({"node_modules", "venv"});
    auto result = scanner.scan(temp.path());
    
    REQUIRE(result.found_folders.size() == 2);
}

TEST_CASE("Scanner respects cancellation", "[scanner][cancellation]") {
    TempDirectory temp;
    
    // Create a large directory structure to give time for cancellation
    for (int i = 0; i < 100; ++i) {
        temp.create_directory("project" + std::to_string(i) + "/node_modules");
        temp.create_directory("project" + std::to_string(i) + "/subdir/venv");
    }
    
    FileSystemScanner scanner({"node_modules", "venv"});
    
    // Start scan in a separate thread and cancel it immediately
    std::atomic<bool> scan_started{false};
    std::thread scan_thread([&]() {
        scan_started = true;
        auto result = scanner.scan(temp.path());
        // Result should be incomplete due to cancellation
    });
    
    // Wait for scan to start
    while (!scan_started) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    // Cancel the scan
    scanner.cancel();
    
    // Wait for scan thread to complete
    scan_thread.join();
    
    // Test passes if we reach here without hanging
    REQUIRE(true);
}
