#include "MainWindow.h"
#include "Logger.h"
#include <QApplication>
#include <QMessageBox>
#include <QStandardPaths>
#include <iostream>
#include <filesystem>

int main(int argc, char* argv[]) {
    try {
        QApplication app(argc, argv);
        
        // Initialize logger for GUI mode
        Logger& logger = Logger::instance();
        logger.set_console_output(false);  // No console output for GUI
        logger.set_min_level(Logger::Level::Info);
        
        // Set log file path in application directory
        try {
            // Use application data directory for GUI mode
            QString app_data_dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            if (app_data_dir.isEmpty()) {
                // Fallback to current directory if app data location not available
                app_data_dir = QString::fromStdString(std::filesystem::current_path().string());
            } else {
                // Ensure the directory exists
                std::filesystem::create_directories(app_data_dir.toStdString());
            }
            
            std::filesystem::path log_path = std::filesystem::path(app_data_dir.toStdString()) / "cleanup.log";
            logger.set_log_file(log_path);
            logger.log(Logger::Level::Info, "Application started in GUI mode");
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not create log file: " << e.what() << std::endl;
        }
        
        std::cout << "Creating MainWindow..." << std::endl;
        MainWindow window;
        std::cout << "MainWindow created, showing window..." << std::endl;
        window.show();
        std::cout << "Window shown, entering event loop..." << std::endl;
        
        int exit_code = app.exec();
        
        // Ensure logger is properly closed before exit
        logger.close_log_file();
        
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
        
        // Show error dialog if possible
        try {
            QMessageBox::critical(nullptr, "Fatal Error", 
                QString("A fatal filesystem error occurred:\n\n%1\n\nPath: %2\n\nThe application will now exit.")
                    .arg(e.what())
                    .arg(QString::fromStdString(e.path1().string())));
        } catch (...) {
            // Ignore dialog errors during fatal error handling
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
        
        // Show error dialog if possible
        try {
            QMessageBox::critical(nullptr, "Fatal Error", 
                QString("A fatal error occurred:\n\n%1\n\nThe application will now exit.")
                    .arg(e.what()));
        } catch (...) {
            // Ignore dialog errors during fatal error handling
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
        
        // Show error dialog if possible
        try {
            QMessageBox::critical(nullptr, "Fatal Error", 
                "An unknown fatal error occurred.\n\nThe application will now exit.");
        } catch (...) {
            // Ignore dialog errors during fatal error handling
        }
        
        return 1;
    }
}
