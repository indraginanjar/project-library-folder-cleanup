#include "SafetyValidator.h"
#include "PathUtils.h"
#include <algorithm>

const std::vector<std::string> SafetyValidator::system_directories_ = {
    // Unix/Linux root and system directories
    "/",
    "/usr",
    "/bin",
    "/sbin",
    "/etc",
    "/var",
    "/lib",
    "/lib64",
    "/boot",
    "/sys",
    "/proc",
    "/dev",
    "/root",
    
    // macOS system directories
    "/System",
    "/Library",
    "/Applications",
    "/Volumes",
    
    // Windows system directories and drive roots
    "C:\\",
    "D:\\",
    "E:\\",
    "F:\\",
    "C:/",
    "D:/",
    "E:/",
    "F:/",
    "/Windows",
    "C:\\Windows",
    "C:/Windows",
    "/Program Files",
    "C:\\Program Files",
    "C:/Program Files",
    "/Program Files (x86)",
    "C:\\Program Files (x86)",
    "C:/Program Files (x86)",
    "C:\\ProgramData",
    "C:/ProgramData"
};

SafetyValidator::ValidationResult SafetyValidator::validate_base_directory(
    const std::filesystem::path& path) {
    
    if (!std::filesystem::exists(path)) {
        return ValidationResult::InvalidPath;
    }
    
    if (!std::filesystem::is_directory(path)) {
        return ValidationResult::InvalidPath;
    }
    
    if (is_system_directory(path)) {
        return ValidationResult::SystemDirectory;
    }
    
    return ValidationResult::Safe;
}

SafetyValidator::ValidationResult SafetyValidator::validate_deletion_target(
    const std::filesystem::path& target,
    const std::filesystem::path& base_directory) {
    
    std::error_code ec;
    
    // Check if target exists
    if (!std::filesystem::exists(target, ec)) {
        return ValidationResult::InvalidPath;
    }
    
    // Get canonical paths (resolves symlinks)
    auto canonical_base = std::filesystem::canonical(base_directory, ec);
    if (ec) {
        return ValidationResult::InvalidPath;
    }
    
    // For the target, we need to check if it's a symlink
    // If it is, we need to verify the link target is within base_directory
    if (std::filesystem::is_symlink(target, ec)) {
        // Read the symlink target
        auto link_target = std::filesystem::read_symlink(target, ec);
        if (ec) {
            // If we can't read the symlink, treat it as unsafe
            return ValidationResult::SymlinkOutsideBase;
        }
        
        // If the link target is relative, resolve it relative to the symlink's parent
        std::filesystem::path resolved_target;
        if (link_target.is_relative()) {
            resolved_target = target.parent_path() / link_target;
        } else {
            resolved_target = link_target;
        }
        
        // Canonicalize the resolved target
        auto canonical_link_target = std::filesystem::canonical(resolved_target, ec);
        if (ec) {
            // If we can't canonicalize, the target might not exist or be inaccessible
            // This is potentially unsafe, so reject it
            return ValidationResult::SymlinkOutsideBase;
        }
        
        // Check if the canonical link target is within the base directory
        auto rel = std::filesystem::relative(canonical_link_target, canonical_base, ec);
        if (ec || rel.empty() || rel.string().find("..") == 0) {
            return ValidationResult::SymlinkOutsideBase;
        }
    }
    
    // For non-symlinks or safe symlinks, verify the target itself is within base
    auto canonical_target = std::filesystem::canonical(target, ec);
    if (ec) {
        return ValidationResult::InvalidPath;
    }
    
    auto rel = std::filesystem::relative(canonical_target, canonical_base, ec);
    if (ec || rel.empty() || rel.string().find("..") == 0) {
        return ValidationResult::SymlinkOutsideBase;
    }
    
    return ValidationResult::Safe;
}

bool SafetyValidator::is_system_directory(const std::filesystem::path& path) {
    std::error_code ec;
    auto canonical = std::filesystem::canonical(path, ec);
    
    // If we can't canonicalize, use the original path
    std::string path_str = ec ? path.string() : canonical.string();
    
    // Check for Windows drive roots (C:\, D:\, etc.)
    if (PathUtils::is_windows_drive_root(path)) {
        return true;
    }
    
    // Check for UNC path roots (like \\server\share)
    if (PathUtils::is_unc_path(path)) {
        // UNC roots are considered system directories for safety
        std::string unc_str = path.string();
        // Count the number of path separators after the initial backslashes
        size_t separator_count = 0;
        for (size_t i = 2; i < unc_str.length(); ++i) {
            if (unc_str[i] == '\\' || unc_str[i] == '/') {
                separator_count++;
            }
        }
        // If there is only one separator (or none), it is a UNC root like \\server or \\server\share
        if (separator_count <= 1) {
            return true;
        }
    }
    
    // Normalize path separators for comparison
    std::replace(path_str.begin(), path_str.end(), '\\', '/');
    
    // Remove trailing slash for comparison
    if (path_str.length() > 1 && path_str.back() == '/') {
        path_str.pop_back();
    }
    
    for (const auto& sys_dir : system_directories_) {
        std::string normalized_sys = sys_dir;
        std::replace(normalized_sys.begin(), normalized_sys.end(), '\\', '/');
        
        // Remove trailing slash from system directory
        if (normalized_sys.length() > 1 && normalized_sys.back() == '/') {
            normalized_sys.pop_back();
        }
        
        // Check for exact match
        if (path_str == normalized_sys) {
            return true;
        }
    }
    
    return false;
}
