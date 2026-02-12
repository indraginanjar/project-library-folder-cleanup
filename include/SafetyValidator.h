#pragma once

#include <filesystem>
#include <vector>
#include <string>

class SafetyValidator {
public:
    enum class ValidationResult {
        Safe,
        SystemDirectory,
        InvalidPath,
        SymlinkOutsideBase
    };

    static ValidationResult validate_base_directory(const std::filesystem::path& path);
    
    static ValidationResult validate_deletion_target(
        const std::filesystem::path& target,
        const std::filesystem::path& base_directory);
    
    static bool is_system_directory(const std::filesystem::path& path);

private:
    static const std::vector<std::string> system_directories_;
};
