#pragma once

#include <filesystem>
#include <string>

/**
 * Utility class for platform-specific path handling
 * Handles Windows and Linux path separators, drive letters, and UNC paths
 */
class PathUtils {
public:
    /**
     * Normalize path separators for the current platform
     * On Windows: converts forward slashes to backslashes
     * On Linux: converts backslashes to forward slashes
     */
    static std::filesystem::path normalize_separators(const std::filesystem::path& path);
    
    /**
     * Check if a path is a Windows drive root (e.g., C:\, D:\)
     * Returns true on Windows for paths like "C:\", "D:/", etc.
     * Returns false on Linux
     */
    static bool is_windows_drive_root(const std::filesystem::path& path);
    
    /**
     * Check if a path is a UNC path (e.g., \\server\share)
     * Returns true for paths starting with \\ or //
     * Only relevant on Windows
     */
    static bool is_unc_path(const std::filesystem::path& path);
    
    /**
     * Get the preferred path separator for the current platform
     * Returns '\' on Windows, '/' on Linux
     */
    static char get_preferred_separator();
    
    /**
     * Convert path to string using platform-specific separators
     */
    static std::string to_platform_string(const std::filesystem::path& path);
    
    /**
     * Check if a path contains a Windows drive letter
     * Returns true for paths like "C:\path" or "D:/path"
     */
    static bool has_drive_letter(const std::filesystem::path& path);
    
    /**
     * Extract drive letter from a Windows path
     * Returns empty string if no drive letter found
     */
    static std::string get_drive_letter(const std::filesystem::path& path);
};
