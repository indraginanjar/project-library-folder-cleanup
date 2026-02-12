#include "PathUtils.h"
#include <algorithm>

#ifdef _WIN32
    #define PLATFORM_SEPARATOR '\\'
    #define IS_WINDOWS true
#else
    #define PLATFORM_SEPARATOR '/'
    #define IS_WINDOWS false
#endif

std::filesystem::path PathUtils::normalize_separators(const std::filesystem::path& path) {
    std::string path_str = path.string();
    
#ifdef _WIN32
    // On Windows, convert forward slashes to backslashes
    std::replace(path_str.begin(), path_str.end(), '/', '\\');
#else
    // On Linux, convert backslashes to forward slashes
    std::replace(path_str.begin(), path_str.end(), '\\', '/');
#endif
    
    return std::filesystem::path(path_str);
}

bool PathUtils::is_windows_drive_root(const std::filesystem::path& path) {
#ifdef _WIN32
    std::string path_str = path.string();
    
    // Remove trailing separators for comparison
    while (!path_str.empty() && (path_str.back() == '\\' || path_str.back() == '/')) {
        path_str.pop_back();
    }
    
    // Check for drive letter format: single letter followed by colon
    // Examples: "C:", "D:", etc.
    if (path_str.length() == 2 && 
        std::isalpha(path_str[0]) && 
        path_str[1] == ':') {
        return true;
    }
    
    return false;
#else
    // On Linux, there are no drive roots
    return false;
#endif
}

bool PathUtils::is_unc_path(const std::filesystem::path& path) {
#ifdef _WIN32
    std::string path_str = path.string();
    
    // UNC paths start with \\ or //
    if (path_str.length() >= 2) {
        if ((path_str[0] == '\\' && path_str[1] == '\\') ||
            (path_str[0] == '/' && path_str[1] == '/')) {
            return true;
        }
    }
    
    return false;
#else
    // UNC paths are Windows-specific
    return false;
#endif
}

char PathUtils::get_preferred_separator() {
    return PLATFORM_SEPARATOR;
}

std::string PathUtils::to_platform_string(const std::filesystem::path& path) {
    std::string path_str = path.string();
    
#ifdef _WIN32
    // Ensure backslashes on Windows
    std::replace(path_str.begin(), path_str.end(), '/', '\\');
#else
    // Ensure forward slashes on Linux
    std::replace(path_str.begin(), path_str.end(), '\\', '/');
#endif
    
    return path_str;
}

bool PathUtils::has_drive_letter(const std::filesystem::path& path) {
    std::string path_str = path.string();
    
    // Check for drive letter at the beginning: letter followed by colon
    if (path_str.length() >= 2 && 
        std::isalpha(path_str[0]) && 
        path_str[1] == ':') {
        return true;
    }
    
    return false;
}

std::string PathUtils::get_drive_letter(const std::filesystem::path& path) {
    std::string path_str = path.string();
    
    // Extract drive letter if present
    if (path_str.length() >= 2 && 
        std::isalpha(path_str[0]) && 
        path_str[1] == ':') {
        return path_str.substr(0, 2);
    }
    
    return "";
}
