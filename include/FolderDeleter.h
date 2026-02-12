#pragma once

#include <filesystem>
#include <vector>
#include <string>
#include <functional>
#include <atomic>
#include <chrono>

class FolderDeleter {
public:
    struct DeletionResult {
        size_t folders_deleted;
        std::uintmax_t space_reclaimed;
        std::vector<std::string> errors;
        std::chrono::milliseconds deletion_duration;
    };

    using ProgressCallback = std::function<void(const std::filesystem::path& current_folder,
                                                  size_t completed,
                                                  size_t total)>;

    enum class Mode {
        DryRun,
        ActualDelete
    };

    FolderDeleter(Mode mode);
    
    DeletionResult delete_folders(const std::vector<std::filesystem::path>& folders,
                                   ProgressCallback progress_callback = nullptr);
    
    void cancel();

private:
    Mode mode_;
    std::atomic<bool> cancelled_;
    
    bool delete_directory_recursive(const std::filesystem::path& path, 
                                     std::string& error_message);
    bool handle_readonly_file(const std::filesystem::path& path);
};
