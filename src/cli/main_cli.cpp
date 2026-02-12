#include "CLIApplication.h"
#include "Logger.h"
#include <iostream>
#include <filesystem>

int main(int argc, char* argv[]) {
    try {
        // Initialize logger early for CLI mode
        // The CLIApplication will configure console output and log level based on verbose flag
        Logger& logger = Logger::instance();
        
        // Set log file path in application directory (current directory for CLI)
        try {
            std::filesystem::path log_path = std::filesystem::current_path() / "cleanup.log";
            logger.set_log_file(log_path);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not create log file: " << e.what() << std::endl;
        }
        
        CLIApplication app(argc, argv);
        int exit_code = app.run();
        
        // Ensure logger is properly closed before exit
        Logger::instance().close_log_file();
        
        return exit_code;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Fatal filesystem error: " << e.what() << std::endl;
        std::cerr << "Path: " << e.path1() << std::endl;
        
        // Attempt to log the error before exiting
        try {
            Logger::instance().log(Logger::Level::Error, 
                std::string("Fatal filesystem error: ") + e.what() + " Path: " + e.path1().string());
            Logger::instance().close_log_file();
        } catch (...) {
            // Ignore logging errors during fatal error handling
        }
        
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        
        // Attempt to log the error before exiting
        try {
            Logger::instance().log(Logger::Level::Error, 
                std::string("Fatal error: ") + e.what());
            Logger::instance().close_log_file();
        } catch (...) {
            // Ignore logging errors during fatal error handling
        }
        
        return 1;
    } catch (...) {
        std::cerr << "Fatal error: Unknown exception occurred" << std::endl;
        
        // Attempt to log the error before exiting
        try {
            Logger::instance().log(Logger::Level::Error, "Fatal error: Unknown exception");
            Logger::instance().close_log_file();
        } catch (...) {
            // Ignore logging errors during fatal error handling
        }
        
        return 1;
    }
}
