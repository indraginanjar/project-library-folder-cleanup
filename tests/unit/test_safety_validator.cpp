#include <catch2/catch_test_macros.hpp>
#include "SafetyValidator.h"

TEST_CASE("SafetyValidator rejects system directories", "[safety]") {
    REQUIRE(SafetyValidator::is_system_directory("/"));
    REQUIRE(SafetyValidator::is_system_directory("/usr"));
    REQUIRE(SafetyValidator::is_system_directory("/System"));
}

TEST_CASE("SafetyValidator accepts normal directories", "[safety]") {
    REQUIRE_FALSE(SafetyValidator::is_system_directory("/home/user/projects"));
    REQUIRE_FALSE(SafetyValidator::is_system_directory("/tmp"));
}

TEST_CASE("SafetyValidator handles Windows drive roots", "[safety][platform][windows]") {
#ifdef _WIN32
    SECTION("rejects Windows drive roots") {
        REQUIRE(SafetyValidator::is_system_directory("C:\\"));
        REQUIRE(SafetyValidator::is_system_directory("C:/"));
        REQUIRE(SafetyValidator::is_system_directory("D:\\"));
        REQUIRE(SafetyValidator::is_system_directory("E:"));
    }
    
    SECTION("accepts non-root Windows paths") {
        REQUIRE_FALSE(SafetyValidator::is_system_directory("C:\\Users\\test"));
        REQUIRE_FALSE(SafetyValidator::is_system_directory("D:\\Projects"));
    }
#endif
}

TEST_CASE("SafetyValidator handles UNC paths", "[safety][platform][windows]") {
#ifdef _WIN32
    SECTION("rejects UNC path roots") {
        // UNC roots like \\server or \\server\share should be rejected
        REQUIRE(SafetyValidator::is_system_directory("\\\\server"));
        REQUIRE(SafetyValidator::is_system_directory("\\\\server\\share"));
    }
    
    SECTION("accepts deeper UNC paths") {
        // Deeper paths within UNC shares should be accepted
        REQUIRE_FALSE(SafetyValidator::is_system_directory("\\\\server\\share\\folder"));
        REQUIRE_FALSE(SafetyValidator::is_system_directory("\\\\server\\share\\projects\\test"));
    }
#endif
}

TEST_CASE("SafetyValidator handles relative paths", "[safety][platform]") {
    SECTION("handles paths with .. components") {
        // Relative paths should not be considered system directories
        REQUIRE_FALSE(SafetyValidator::is_system_directory("../relative/path"));
        REQUIRE_FALSE(SafetyValidator::is_system_directory("./current/path"));
    }
}
