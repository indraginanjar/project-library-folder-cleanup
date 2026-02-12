#pragma once

#include "ApplicationController.h"
#include <filesystem>
#include <vector>
#include <string>

class CLIApplication {
public:
    struct CLIOptions {
        std::filesystem::path base_directory;
        std::vector<std::string> custom_folders;
        bool dry_run;
        bool force;
        bool verbose;
        bool help;
    };

    CLIApplication(int argc, char* argv[]);
    
    int run();

private:
    CLIOptions options_;
    ApplicationController controller_;
    
    bool parse_arguments(int argc, char* argv[]);
    void print_usage();
    void print_scan_results(const FileSystemScanner::ScanResult& result);
    void print_deletion_results(const FolderDeleter::DeletionResult& result);
    bool prompt_confirmation();
};
