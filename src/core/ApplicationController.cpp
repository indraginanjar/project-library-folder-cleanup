#include "ApplicationController.h"
#include "SafetyValidator.h"

ApplicationController::ApplicationController() 
    : logger_(Logger::instance()) {
}

bool ApplicationController::execute_scan(
    const OperationConfig& config,
    FileSystemScanner::ProgressCallback progress_callback) {
    
    // Validate base directory using SafetyValidator
    auto validation_result = SafetyValidator::validate_base_directory(config.base_directory);
    
    if (validation_result == SafetyValidator::ValidationResult::SystemDirectory) {
        logger_.log(Logger::Level::Error, 
                   "Cannot scan system directory: " + config.base_directory.string());
        return false;
    }
    
    if (validation_result == SafetyValidator::ValidationResult::InvalidPath) {
        logger_.log(Logger::Level::Error, 
                   "Invalid base directory path: " + config.base_directory.string());
        return false;
    }
    
    // Log scan operation start
    logger_.log(Logger::Level::Info, 
               "Starting scan of directory: " + config.base_directory.string());
    
    // Create scanner and execute scan
    scanner_ = std::make_unique<FileSystemScanner>(config.target_folders);
    scan_result_ = scanner_->scan(config.base_directory, progress_callback);
    
    // Log scan results
    logger_.log(Logger::Level::Info, 
               "Scan complete. Found " + std::to_string(scan_result_.found_folders.size()) + 
               " folders, total size: " + std::to_string(scan_result_.total_size) + " bytes");
    
    // Log any errors encountered during scan
    for (const auto& error : scan_result_.errors) {
        logger_.log(Logger::Level::Warning, "Scan error: " + error);
    }
    
    // Create deleter for subsequent deletion operation
    deleter_ = std::make_unique<FolderDeleter>(config.deletion_mode);
    
    return true;
}

bool ApplicationController::execute_deletion(
    FolderDeleter::ProgressCallback progress_callback) {
    
    if (!deleter_) {
        logger_.log(Logger::Level::Error, 
                   "Cannot execute deletion: no scan has been performed");
        return false;
    }
    
    if (scan_result_.found_folders.empty()) {
        logger_.log(Logger::Level::Info, "No folders to delete");
        return true;
    }
    
    // Log deletion operation start
    logger_.log(Logger::Level::Info, 
               "Starting deletion of " + std::to_string(scan_result_.found_folders.size()) + " folders");
    
    // Execute deletion
    deletion_result_ = deleter_->delete_folders(scan_result_.found_folders, progress_callback);
    
    // Log deletion results
    logger_.log(Logger::Level::Info, 
               "Deletion complete. Deleted " + std::to_string(deletion_result_.folders_deleted) + 
               " folders, reclaimed " + std::to_string(deletion_result_.space_reclaimed) + " bytes");
    
    // Log any errors encountered during deletion
    for (const auto& error : deletion_result_.errors) {
        logger_.log(Logger::Level::Warning, "Deletion error: " + error);
    }
    
    return true;
}

const FileSystemScanner::ScanResult& ApplicationController::get_scan_result() const {
    return scan_result_;
}

const FolderDeleter::DeletionResult& ApplicationController::get_deletion_result() const {
    return deletion_result_;
}

void ApplicationController::cancel_operation() {
    logger_.log(Logger::Level::Info, "Cancelling operation");
    
    if (scanner_) {
        scanner_->cancel();
    }
    if (deleter_) {
        deleter_->cancel();
    }
}
