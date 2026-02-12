#pragma once

#include <filesystem>
#include <vector>
#include <string>
#include <functional>
#include <atomic>
#include <chrono>
#include <unordered_map>

class FileSystemScanner {
public:
    struct ScanResult {
        std::vector<std::filesystem::path> found_folders;
        std::uintmax_t total_size;
        std::vector<std::string> errors;
        std::chrono::milliseconds scan_duration;
    };

    using ProgressCallback = std::function<void(const std::string& current_path, 
                                                  size_t processed, 
                                                  size_t total)>;

    FileSystemScanner(const std::vector<std::string>& target_folder_names);
    
    ScanResult scan(const std::filesystem::path& base_directory,
                    ProgressCallback progress_callback = nullptr);
    
    void cancel();

private:
    std::vector<std::string> target_folder_names_;
    std::atomic<bool> cancelled_;
    std::unordered_map<std::string, std::uintmax_t> size_cache_;
    
    bool is_target_folder(const std::filesystem::path& path) const;
    std::uintmax_t calculate_directory_size(const std::filesystem::path& path);
    void scan_recursive(const std::filesystem::path& current_path,
                       ScanResult& result,
                       size_t& processed_count,
                       ProgressCallback& progress_callback);
};
