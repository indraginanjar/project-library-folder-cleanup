#include "FileSystemScanner.h"
#include "Logger.h"
#include <algorithm>

FileSystemScanner::FileSystemScanner(const std::vector<std::string>& target_folder_names)
    : target_folder_names_(target_folder_names)
    , cancelled_(false) {
}

FileSystemScanner::ScanResult FileSystemScanner::scan(
    const std::filesystem::path& base_directory,
    ProgressCallback progress_callback) {
    
    auto start_time = std::chrono::steady_clock::now();
    
    ScanResult result;
    result.total_size = 0;
    
    // Clear the size cache for this scan
    size_cache_.clear();
    
    // Reset cancellation flag
    cancelled_ = false;
    
    // Validate base directory
    if (!std::filesystem::exists(base_directory)) {
        result.errors.push_back("Base directory does not exist: " + base_directory.string());
        Logger::instance().log(Logger::Level::Error, "Base directory does not exist: " + base_directory.string());
        result.scan_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start_time);
        return result;
    }
    
    if (!std::filesystem::is_directory(base_directory)) {
        result.errors.push_back("Base path is not a directory: " + base_directory.string());
        Logger::instance().log(Logger::Level::Error, "Base path is not a directory: " + base_directory.string());
        result.scan_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start_time);
        return result;
    }
    
    Logger::instance().log(Logger::Level::Info, "Starting scan of: " + base_directory.string());
    
    size_t processed_count = 0;
    scan_recursive(base_directory, result, processed_count, progress_callback);
    
    // Calculate total size of all found folders
    for (const auto& folder : result.found_folders) {
        if (cancelled_) {
            break;
        }
        result.total_size += calculate_directory_size(folder);
    }
    
    auto end_time = std::chrono::steady_clock::now();
    result.scan_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    Logger::instance().log(Logger::Level::Info, 
        "Scan complete. Found " + std::to_string(result.found_folders.size()) + 
        " folders, total size: " + std::to_string(result.total_size) + " bytes");
    
    return result;
}

void FileSystemScanner::scan_recursive(
    const std::filesystem::path& current_path,
    ScanResult& result,
    size_t& processed_count,
    ProgressCallback& progress_callback) {
    
    if (cancelled_) {
        return;
    }
    
    try {
        // Check if current directory is a target folder
        if (is_target_folder(current_path)) {
            result.found_folders.push_back(current_path);
            Logger::instance().log(Logger::Level::Info, "Found target folder: " + current_path.string());
            
            // Don't recurse into target folders - we want to delete them entirely
            return;
        }
        
        // Iterate through directory entries
        for (const auto& entry : std::filesystem::directory_iterator(current_path)) {
            if (cancelled_) {
                return;
            }
            
            processed_count++;
            
            // Report progress every 100 items
            if (progress_callback && processed_count % 100 == 0) {
                progress_callback(entry.path().string(), processed_count, 0);
            }
            
            // Recurse into subdirectories
            if (entry.is_directory()) {
                scan_recursive(entry.path(), result, processed_count, progress_callback);
            }
        }
        
    } catch (const std::filesystem::filesystem_error& e) {
        // Handle permission errors and other filesystem errors gracefully
        std::string error_msg = "Error accessing " + current_path.string() + ": " + e.what();
        result.errors.push_back(error_msg);
        Logger::instance().log(Logger::Level::Warning, error_msg);
        // Continue scanning other directories
    } catch (const std::exception& e) {
        std::string error_msg = "Unexpected error at " + current_path.string() + ": " + e.what();
        result.errors.push_back(error_msg);
        Logger::instance().log(Logger::Level::Error, error_msg);
    }
}

void FileSystemScanner::cancel() {
    cancelled_ = true;
    Logger::instance().log(Logger::Level::Info, "Scan cancellation requested");
}

bool FileSystemScanner::is_target_folder(const std::filesystem::path& path) const {
    std::string folder_name = path.filename().string();
    
    for (const auto& target : target_folder_names_) {
        if (folder_name == target) {
            return true;
        }
    }
    
    return false;
}

std::uintmax_t FileSystemScanner::calculate_directory_size(const std::filesystem::path& path) {
    // Check cache first
    std::string path_str = path.string();
    auto cache_it = size_cache_.find(path_str);
    if (cache_it != size_cache_.end()) {
        return cache_it->second;
    }
    
    std::uintmax_t size = 0;
    
    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(
                path, 
                std::filesystem::directory_options::skip_permission_denied)) {
            
            if (cancelled_) {
                break;
            }
            
            try {
                if (entry.is_regular_file()) {
                    size += entry.file_size();
                }
            } catch (const std::filesystem::filesystem_error& e) {
                // Skip files we can't access
                Logger::instance().log(Logger::Level::Debug, 
                    "Could not get size of file: " + entry.path().string());
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        // Log error but return partial size
        Logger::instance().log(Logger::Level::Warning, 
            "Error calculating size for " + path.string() + ": " + e.what());
    } catch (const std::exception& e) {
        Logger::instance().log(Logger::Level::Error, 
            "Unexpected error calculating size for " + path.string() + ": " + e.what());
    }
    
    // Cache the result
    size_cache_[path_str] = size;
    
    return size;
}
