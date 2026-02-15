#include "CLIApplication.h"
#include "ConfigurationManager.h"
#include "SafetyValidator.h"
#include "Logger.h"
#include "Version.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>

CLIApplication::CLIApplication(int argc, char* argv[]) {
    // Initialize default options
    options_.dry_run = false;
    options_.force = false;
    options_.verbose = false;
    options_.help = false;
    
    if (!parse_arguments(argc, argv)) {
        options_.help = true;
    }
}

int CLIApplication::run() {
    // Show help if requested
    if (options_.help) {
        print_usage();
        return 0;
    }
    
    // Configure logger based on verbose flag
    Logger& logger = Logger::instance();
    logger.set_console_output(options_.verbose);
    logger.set_min_level(options_.verbose ? Logger::Level::Debug : Logger::Level::Info);
    
    logger.log(Logger::Level::Info, "Application started in CLI mode");
    
    // Validate base directory
    if (options_.base_directory.empty()) {
        std::cerr << "Error: Base directory is required" << std::endl;
        print_usage();
        logger.log(Logger::Level::Error, "Base directory not provided");
        return 1;
    }
    
    // Check if base directory exists
    if (!std::filesystem::exists(options_.base_directory)) {
        std::cerr << "Error: Base directory does not exist: " 
                  << options_.base_directory << std::endl;
        logger.log(Logger::Level::Error, 
                  "Base directory does not exist: " + options_.base_directory.string());
        return 1;
    }
    
    // Validate base directory is not a system directory
    auto validation_result = SafetyValidator::validate_base_directory(options_.base_directory);
    if (validation_result == SafetyValidator::ValidationResult::SystemDirectory) {
        std::cerr << "Error: Cannot operate on system directory: " 
                  << options_.base_directory << std::endl;
        logger.log(Logger::Level::Error, 
                  "Attempted to operate on system directory: " + options_.base_directory.string());
        return 1;
    }
    if (validation_result == SafetyValidator::ValidationResult::InvalidPath) {
        std::cerr << "Error: Invalid base directory path: " 
                  << options_.base_directory << std::endl;
        logger.log(Logger::Level::Error, 
                  "Invalid base directory path: " + options_.base_directory.string());
        return 1;
    }
    
    // Determine target folders
    std::vector<std::string> target_folders;
    if (!options_.custom_folders.empty()) {
        target_folders = options_.custom_folders;
    } else {
        target_folders = ConfigurationManager::get_default_folders();
    }
    
    if (options_.verbose) {
        std::cout << "Target folders: ";
        for (size_t i = 0; i < target_folders.size(); ++i) {
            std::cout << target_folders[i];
            if (i < target_folders.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }
    
    // Configure operation
    ApplicationController::OperationConfig config;
    config.base_directory = options_.base_directory;
    config.target_folders = target_folders;
    config.deletion_mode = options_.dry_run ? FolderDeleter::Mode::DryRun 
                                             : FolderDeleter::Mode::ActualDelete;
    config.require_confirmation = !options_.force;
    
    // Execute scan
    std::cout << "Scanning directory: " << options_.base_directory << std::endl;
    
    auto scan_progress = [this](const std::string& current_path, size_t processed, size_t total) {
        if (options_.verbose) {
            std::cout << "Scanning: " << current_path << std::endl;
        }
    };
    
    bool scan_success = false;
    try {
        scan_success = controller_.execute_scan(config, scan_progress);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Fatal filesystem error during scan: " << e.what() << std::endl;
        std::cerr << "Path: " << e.path1() << std::endl;
        logger.log(Logger::Level::Error, 
                  std::string("Fatal filesystem error during scan: ") + e.what() + 
                  " Path: " + e.path1().string());
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error during scan: " << e.what() << std::endl;
        logger.log(Logger::Level::Error, 
                  std::string("Fatal error during scan: ") + e.what());
        return 1;
    }
    
    if (!scan_success) {
        std::cerr << "Error: Scan operation failed" << std::endl;
        logger.log(Logger::Level::Error, "Scan operation failed");
        return 1;
    }
    
    // Print scan results
    const auto& scan_result = controller_.get_scan_result();
    print_scan_results(scan_result);
    
    // If no folders found, exit successfully
    if (scan_result.found_folders.empty()) {
        std::cout << "No library folders found." << std::endl;
        logger.log(Logger::Level::Info, "No library folders found");
        return 0;
    }
    
    // If dry-run mode, don't proceed with deletion
    if (options_.dry_run) {
        std::cout << "\nDry-run mode: No folders were deleted." << std::endl;
        logger.log(Logger::Level::Info, "Dry-run mode completed");
        return 0;
    }
    
    // Prompt for confirmation if force flag not set
    if (!options_.force) {
        if (!prompt_confirmation()) {
            std::cout << "Operation cancelled by user." << std::endl;
            logger.log(Logger::Level::Info, "Operation cancelled by user");
            return 0;
        }
    }
    
    // Execute deletion
    std::cout << "\nDeleting folders..." << std::endl;
    
    auto delete_progress = [this](const std::filesystem::path& current_folder, 
                                   size_t completed, size_t total) {
        std::cout << "Deleting [" << (completed + 1) << "/" << total << "]: " 
                  << current_folder << std::endl;
    };
    
    bool delete_success = false;
    try {
        delete_success = controller_.execute_deletion(delete_progress);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Fatal filesystem error during deletion: " << e.what() << std::endl;
        std::cerr << "Path: " << e.path1() << std::endl;
        logger.log(Logger::Level::Error, 
                  std::string("Fatal filesystem error during deletion: ") + e.what() + 
                  " Path: " + e.path1().string());
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error during deletion: " << e.what() << std::endl;
        logger.log(Logger::Level::Error, 
                  std::string("Fatal error during deletion: ") + e.what());
        return 1;
    }
    
    if (!delete_success) {
        std::cerr << "Error: Deletion operation failed" << std::endl;
        logger.log(Logger::Level::Error, "Deletion operation failed");
        return 1;
    }
    
    // Print deletion results
    const auto& deletion_result = controller_.get_deletion_result();
    print_deletion_results(deletion_result);
    
    // Return success if no errors, or partial success code if some deletions failed
    if (!deletion_result.errors.empty()) {
        logger.log(Logger::Level::Warning, 
                  "Deletion completed with " + std::to_string(deletion_result.errors.size()) + " errors");
        return 2; // Partial success
    }
    
    logger.log(Logger::Level::Info, "Operation completed successfully");
    return 0;
}

bool CLIApplication::parse_arguments(int argc, char* argv[]) {
    if (argc < 2) {
        return false;
    }
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            options_.help = true;
            return true;
        }
        else if (arg == "--version") {
            std::cout << Version::get_version_string() << std::endl;
            exit(0);
        }
        else if (arg == "--dry-run") {
            options_.dry_run = true;
        }
        else if (arg == "--force" || arg == "-f") {
            options_.force = true;
        }
        else if (arg == "--verbose" || arg == "-v") {
            options_.verbose = true;
        }
        else if (arg == "--folders") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --folders requires an argument" << std::endl;
                return false;
            }
            ++i;
            std::string folders_str = argv[i];
            
            // Parse comma-separated list
            std::stringstream ss(folders_str);
            std::string folder;
            while (std::getline(ss, folder, ',')) {
                // Trim whitespace
                folder.erase(0, folder.find_first_not_of(" \t"));
                folder.erase(folder.find_last_not_of(" \t") + 1);
                if (!folder.empty()) {
                    options_.custom_folders.push_back(folder);
                }
            }
        }
        else if (arg[0] == '-') {
            std::cerr << "Error: Unknown option: " << arg << std::endl;
            return false;
        }
        else {
            // Assume it's the base directory
            if (!options_.base_directory.empty()) {
                std::cerr << "Error: Multiple base directories specified" << std::endl;
                return false;
            }
            options_.base_directory = arg;
        }
    }
    
    // Base directory is required unless help is requested
    if (!options_.help && options_.base_directory.empty()) {
        std::cerr << "Error: Base directory is required" << std::endl;
        return false;
    }
    
    return true;
}

void CLIApplication::print_usage() {
    std::cout << Version::get_version_string() << "\n\n"
              << "Usage: cleanup_cli [options] <base_directory>\n\n"
              << "Arguments:\n"
              << "  <base_directory>   Root directory to scan recursively\n\n"
              << "Options:\n"
              << "  --dry-run          Scan only, do not delete folders\n"
              << "  --force, -f        Skip confirmation prompt before deletion\n"
              << "  --verbose, -v      Enable verbose output and detailed logging\n"
              << "  --folders <list>   Comma-separated list of folder names to target\n"
              << "                     (default: node_modules,venv,.venv,target,.gradle,build,dist,__pycache__)\n"
              << "  --version          Show version information\n"
              << "  --help, -h         Show this help message\n\n"
              << "Examples:\n"
              << "  cleanup_cli ~/projects                    # Scan and delete with confirmation\n"
              << "  cleanup_cli --dry-run ~/projects          # Scan only, no deletion\n"
              << "  cleanup_cli --force ~/projects            # Delete without confirmation\n"
              << "  cleanup_cli --folders node_modules,dist ~/projects  # Custom folder list\n"
              << "  cleanup_cli --verbose ~/projects          # Show detailed progress\n\n"
              << "Exit Codes:\n"
              << "  0 - Success\n"
              << "  1 - Fatal error (invalid arguments, system directory, scan failed)\n"
              << "  2 - Partial success (some folders could not be deleted)\n";
}

void CLIApplication::print_scan_results(const FileSystemScanner::ScanResult& result) {
    std::cout << "\n=== Scan Results ===" << std::endl;
    std::cout << "Found " << result.found_folders.size() << " library folder(s)" << std::endl;
    
    // Convert bytes to human-readable format
    auto format_size = [](std::uintmax_t bytes) -> std::string {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unit_index = 0;
        double size = static_cast<double>(bytes);
        
        while (size >= 1024.0 && unit_index < 4) {
            size /= 1024.0;
            ++unit_index;
        }
        
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << size << " " << units[unit_index];
        return oss.str();
    };
    
    std::cout << "Total size: " << format_size(result.total_size) 
              << " (" << result.total_size << " bytes)" << std::endl;
    std::cout << "Scan duration: " << result.scan_duration.count() << " ms" << std::endl;
    
    if (!result.found_folders.empty()) {
        std::cout << "\nFound folders:" << std::endl;
        for (const auto& folder : result.found_folders) {
            std::cout << "  " << folder << std::endl;
        }
    }
    
    if (!result.errors.empty()) {
        std::cerr << "\nErrors encountered during scan:" << std::endl;
        for (const auto& error : result.errors) {
            std::cerr << "  " << error << std::endl;
        }
    }
}

void CLIApplication::print_deletion_results(const FolderDeleter::DeletionResult& result) {
    std::cout << "\n=== Deletion Results ===" << std::endl;
    std::cout << "Deleted " << result.folders_deleted << " folder(s)" << std::endl;
    
    // Convert bytes to human-readable format
    auto format_size = [](std::uintmax_t bytes) -> std::string {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unit_index = 0;
        double size = static_cast<double>(bytes);
        
        while (size >= 1024.0 && unit_index < 4) {
            size /= 1024.0;
            ++unit_index;
        }
        
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << size << " " << units[unit_index];
        return oss.str();
    };
    
    std::cout << "Space reclaimed: " << format_size(result.space_reclaimed) 
              << " (" << result.space_reclaimed << " bytes)" << std::endl;
    std::cout << "Deletion duration: " << result.deletion_duration.count() << " ms" << std::endl;
    
    if (!result.errors.empty()) {
        std::cerr << "\nErrors encountered during deletion:" << std::endl;
        for (const auto& error : result.errors) {
            std::cerr << "  " << error << std::endl;
        }
    }
    
    if (result.errors.empty()) {
        std::cout << "\nAll folders deleted successfully!" << std::endl;
    } else {
        std::cout << "\nDeletion completed with some errors." << std::endl;
    }
}

bool CLIApplication::prompt_confirmation() {
    std::cout << "\nProceed with deletion? (y/n): ";
    std::string response;
    std::getline(std::cin, response);
    
    // Trim whitespace
    response.erase(0, response.find_first_not_of(" \t\n\r"));
    response.erase(response.find_last_not_of(" \t\n\r") + 1);
    
    return response == "y" || response == "Y" || response == "yes" || response == "Yes" || response == "YES";
}
