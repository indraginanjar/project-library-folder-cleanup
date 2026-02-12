#include "FolderDeleter.h"
#include "Logger.h"
#include <system_error>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

FolderDeleter::FolderDeleter(Mode mode)
    : mode_(mode)
    , cancelled_(false) {
}

FolderDeleter::DeletionResult FolderDeleter::delete_folders(
    const std::vector<std::filesystem::path>& folders,
    ProgressCallback progress_callback) {
    
    DeletionResult result;
    result.folders_deleted = 0;
    result.space_reclaimed = 0;
    
    auto start_time = std::chrono::steady_clock::now();
    
    for (size_t i = 0; i < folders.size() && !cancelled_; ++i) {
        const auto& folder = folders[i];
        
        if (progress_callback) {
            progress_callback(folder, i, folders.size());
        }
        
        // Calculate size before deletion
        std::uintmax_t folder_size = 0;
        try {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(folder)) {
                if (entry.is_regular_file()) {
                    folder_size += entry.file_size();
                }
            }
        } catch (const std::exception&) {
            // Ignore errors during size calculation
        }
        
        // Delete the folder
        std::string error_message;
        if (delete_directory_recursive(folder, error_message)) {
            result.folders_deleted++;
            result.space_reclaimed += folder_size;
        } else {
            result.errors.push_back("Failed to delete " + folder.string() + ": " + error_message);
        }
    }
    
    auto end_time = std::chrono::steady_clock::now();
    result.deletion_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    return result;
}

void FolderDeleter::cancel() {
    cancelled_ = true;
}

bool FolderDeleter::delete_directory_recursive(
    const std::filesystem::path& path,
    std::string& error_message) {
    
    if (mode_ == Mode::DryRun) {
        // In dry-run mode, just verify the path exists
        if (!std::filesystem::exists(path)) {
            error_message = "Path does not exist";
            return false;
        }
        return true;
    }
    
    try {
        // Attempt to remove read-only attributes from all files in the directory
        try {
            for (auto& entry : std::filesystem::recursive_directory_iterator(
                path, std::filesystem::directory_options::skip_permission_denied)) {
                
                if (cancelled_) {
                    error_message = "Operation cancelled";
                    return false;
                }
                
                if (entry.is_regular_file() || entry.is_directory()) {
                    handle_readonly_file(entry.path());
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            // Log but continue - we'll try to delete anyway
            Logger::instance().log(Logger::Level::Warning, 
                "Error clearing read-only attributes: " + std::string(e.what()));
        }
        
        // Now attempt to delete the entire directory tree
        std::error_code ec;
        std::uintmax_t removed = std::filesystem::remove_all(path, ec);
        
        if (ec) {
            error_message = ec.message();
            Logger::instance().log(Logger::Level::Error, 
                "Failed to delete " + path.string() + ": " + error_message);
            return false;
        }
        
        if (removed == static_cast<std::uintmax_t>(-1)) {
            error_message = "Path does not exist or could not be accessed";
            return false;
        }
        
        Logger::instance().log(Logger::Level::Info, 
            "Successfully deleted " + path.string() + " (" + std::to_string(removed) + " items)");
        return true;
        
    } catch (const std::exception& e) {
        error_message = e.what();
        Logger::instance().log(Logger::Level::Error, 
            "Exception during deletion of " + path.string() + ": " + error_message);
        return false;
    }
}

bool FolderDeleter::handle_readonly_file(const std::filesystem::path& path) {
    try {
#ifdef _WIN32
        // Windows: Clear read-only, hidden, and system attributes
        std::wstring wide_path = path.wstring();
        DWORD attributes = GetFileAttributesW(wide_path.c_str());
        
        if (attributes == INVALID_FILE_ATTRIBUTES) {
            return false;
        }
        
        // Remove read-only, hidden, and system attributes
        DWORD new_attributes = attributes & ~(FILE_ATTRIBUTE_READONLY | 
                                              FILE_ATTRIBUTE_HIDDEN | 
                                              FILE_ATTRIBUTE_SYSTEM);
        
        if (new_attributes != attributes) {
            if (!SetFileAttributesW(wide_path.c_str(), new_attributes)) {
                Logger::instance().log(Logger::Level::Warning, 
                    "Failed to clear attributes for: " + path.string());
                return false;
            }
        }
        return true;
#else
        // Linux/Unix: Add write permissions
        auto perms = std::filesystem::status(path).permissions();
        
        // Add write permission for owner, group, and others
        std::filesystem::permissions(path, 
            perms | std::filesystem::perms::owner_write |
                    std::filesystem::perms::group_write |
                    std::filesystem::perms::others_write,
            std::filesystem::perm_options::add);
        
        return true;
#endif
    } catch (const std::exception& e) {
        Logger::instance().log(Logger::Level::Warning, 
            "Exception clearing read-only for " + path.string() + ": " + e.what());
        return false;
    }
}
