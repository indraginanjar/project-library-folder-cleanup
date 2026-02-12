#include <catch2/catch_test_macros.hpp>
#include "PathUtils.h"

TEST_CASE("PathUtils handles path separators correctly", "[path][platform]") {
    SECTION("normalize_separators converts to platform-specific format") {
        std::filesystem::path mixed_path = "C:/Users\\test/folder\\file.txt";
        auto normalized = PathUtils::normalize_separators(mixed_path);
        
        // The normalized path should use the platform's preferred separator
        std::string normalized_str = normalized.string();
        
#ifdef _WIN32
        // On Windows, should have backslashes (or at least not only forward slashes)
        bool has_backslash = normalized_str.find('\\') != std::string::npos;
        bool has_forward = normalized_str.find('/') != std::string::npos;
        REQUIRE((has_backslash || !has_forward));
#else
        // On Linux, should not have backslashes
        REQUIRE(normalized_str.find('\\') == std::string::npos);
#endif
    }
    
    SECTION("get_preferred_separator returns correct separator") {
        char sep = PathUtils::get_preferred_separator();
        
#ifdef _WIN32
        REQUIRE(sep == '\\');
#else
        REQUIRE(sep == '/');
#endif
    }
}

TEST_CASE("PathUtils handles Windows drive letters", "[path][platform][windows]") {
    SECTION("has_drive_letter detects drive letters") {
        REQUIRE(PathUtils::has_drive_letter("C:\\Users"));
        REQUIRE(PathUtils::has_drive_letter("D:/Projects"));
        REQUIRE(PathUtils::has_drive_letter("E:\\"));
        REQUIRE_FALSE(PathUtils::has_drive_letter("/home/user"));
        REQUIRE_FALSE(PathUtils::has_drive_letter("relative/path"));
    }
    
    SECTION("get_drive_letter extracts drive letter") {
        REQUIRE(PathUtils::get_drive_letter("C:\\Users") == "C:");
        REQUIRE(PathUtils::get_drive_letter("D:/Projects") == "D:");
        REQUIRE(PathUtils::get_drive_letter("E:\\") == "E:");
        REQUIRE(PathUtils::get_drive_letter("/home/user") == "");
        REQUIRE(PathUtils::get_drive_letter("relative/path") == "");
    }
    
    SECTION("is_windows_drive_root detects drive roots") {
#ifdef _WIN32
        REQUIRE(PathUtils::is_windows_drive_root("C:\\"));
        REQUIRE(PathUtils::is_windows_drive_root("C:/"));
        REQUIRE(PathUtils::is_windows_drive_root("D:\\"));
        REQUIRE(PathUtils::is_windows_drive_root("E:"));
        REQUIRE_FALSE(PathUtils::is_windows_drive_root("C:\\Users"));
        REQUIRE_FALSE(PathUtils::is_windows_drive_root("C:\\Windows\\System32"));
#else
        // On Linux, no paths are Windows drive roots
        REQUIRE_FALSE(PathUtils::is_windows_drive_root("C:\\"));
        REQUIRE_FALSE(PathUtils::is_windows_drive_root("D:/"));
#endif
    }
}

TEST_CASE("PathUtils handles UNC paths", "[path][platform][windows]") {
    SECTION("is_unc_path detects UNC paths") {
#ifdef _WIN32
        REQUIRE(PathUtils::is_unc_path("\\\\server\\share"));
        REQUIRE(PathUtils::is_unc_path("//server/share"));
        REQUIRE(PathUtils::is_unc_path("\\\\server\\share\\folder"));
        REQUIRE_FALSE(PathUtils::is_unc_path("C:\\Users"));
        REQUIRE_FALSE(PathUtils::is_unc_path("/home/user"));
        REQUIRE_FALSE(PathUtils::is_unc_path("\\single\\backslash"));
#else
        // On Linux, UNC paths are not recognized
        REQUIRE_FALSE(PathUtils::is_unc_path("\\\\server\\share"));
        REQUIRE_FALSE(PathUtils::is_unc_path("//server/share"));
#endif
    }
}

TEST_CASE("PathUtils to_platform_string converts correctly", "[path][platform]") {
    SECTION("converts mixed separators to platform format") {
        std::filesystem::path path1 = "folder/subfolder\\file.txt";
        std::string platform_str = PathUtils::to_platform_string(path1);
        
#ifdef _WIN32
        // On Windows, should use backslashes (or at least not only forward slashes)
        bool has_backslash = platform_str.find('\\') != std::string::npos;
        bool has_forward = platform_str.find('/') != std::string::npos;
        REQUIRE((has_backslash || !has_forward));
#else
        // On Linux, should not have backslashes
        REQUIRE(platform_str.find('\\') == std::string::npos);
#endif
    }
}

TEST_CASE("PathUtils handles edge cases", "[path][platform]") {
    SECTION("handles empty paths") {
        std::filesystem::path empty_path;
        REQUIRE_FALSE(PathUtils::has_drive_letter(empty_path));
        REQUIRE(PathUtils::get_drive_letter(empty_path) == "");
        REQUIRE_FALSE(PathUtils::is_unc_path(empty_path));
    }
    
    SECTION("handles single character paths") {
        REQUIRE_FALSE(PathUtils::has_drive_letter("C"));
        REQUIRE_FALSE(PathUtils::has_drive_letter("/"));
        REQUIRE_FALSE(PathUtils::has_drive_letter("\\"));
    }
    
    SECTION("handles relative paths") {
        std::filesystem::path rel_path = "relative/path/to/folder";
        REQUIRE_FALSE(PathUtils::has_drive_letter(rel_path));
        REQUIRE_FALSE(PathUtils::is_unc_path(rel_path));
    }
}
