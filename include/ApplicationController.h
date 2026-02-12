#pragma once

#include "FileSystemScanner.h"
#include "FolderDeleter.h"
#include "SafetyValidator.h"
#include "Logger.h"
#include <memory>
#include <filesystem>

class ApplicationController {
public:
    struct OperationConfig {
        std::filesystem::path base_directory;
        std::vector<std::string> target_folders;
        FolderDeleter::Mode deletion_mode;
        bool require_confirmation;
    };

    ApplicationController();
    
    bool execute_scan(const OperationConfig& config,
                      FileSystemScanner::ProgressCallback progress_callback);
    
    bool execute_deletion(FolderDeleter::ProgressCallback progress_callback);
    
    const FileSystemScanner::ScanResult& get_scan_result() const;
    const FolderDeleter::DeletionResult& get_deletion_result() const;
    
    void cancel_operation();

private:
    std::unique_ptr<FileSystemScanner> scanner_;
    std::unique_ptr<FolderDeleter> deleter_;
    FileSystemScanner::ScanResult scan_result_;
    FolderDeleter::DeletionResult deletion_result_;
    Logger& logger_;
};
