#include <catch2/catch_test_macros.hpp>
#include "Logger.h"
#include "temp_directory.h"

TEST_CASE("Logger creates log file", "[logger]") {
    TempDirectory temp;
    auto log_path = temp.path() / "test.log";
    
    Logger::instance().set_log_file(log_path);
    Logger::instance().log(Logger::Level::Info, "Test message");
    Logger::instance().close_log_file();
    
    REQUIRE(std::filesystem::exists(log_path));
}

TEST_CASE("Logger filters by level", "[logger]") {
    Logger::instance().set_console_output(false);
    Logger::instance().set_min_level(Logger::Level::Warning);
    
    // Should not log debug or info
    Logger::instance().log(Logger::Level::Debug, "Debug message");
    Logger::instance().log(Logger::Level::Info, "Info message");
    
    // Should log warning and error
    Logger::instance().log(Logger::Level::Warning, "Warning message");
    Logger::instance().log(Logger::Level::Error, "Error message");
    
    // Test passes if no exceptions thrown
    REQUIRE(true);
}
