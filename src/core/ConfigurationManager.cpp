#include "ConfigurationManager.h"
#include "Logger.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>

using json = nlohmann::json;

ConfigurationManager& ConfigurationManager::instance() {
    static ConfigurationManager instance;
    return instance;
}

ConfigurationManager::ConfigurationManager() {
    target_folders_ = get_default_folders();
}

std::vector<std::string> ConfigurationManager::get_target_folders() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return target_folders_;
}

void ConfigurationManager::set_target_folders(const std::vector<std::string>& folders) {
    std::lock_guard<std::mutex> lock(mutex_);
    target_folders_.clear();
    
    for (const auto& folder : folders) {
        if (validate_folder_name(folder)) {
            target_folders_.push_back(folder);
        }
    }
}

bool ConfigurationManager::load_from_file(const std::filesystem::path& config_path) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        // Check if file exists
        if (!std::filesystem::exists(config_path)) {
            Logger::instance().log(Logger::Level::Warning, 
                "Configuration file not found: " + config_path.string());
            return false;
        }
        
        // Open and parse JSON file
        std::ifstream file(config_path);
        if (!file.is_open()) {
            Logger::instance().log(Logger::Level::Error, 
                "Failed to open configuration file: " + config_path.string());
            return false;
        }
        
        json config;
        file >> config;
        
        // Validate JSON structure
        if (!config.contains("target_folders") || !config["target_folders"].is_array()) {
            Logger::instance().log(Logger::Level::Error, 
                "Invalid configuration format: missing or invalid 'target_folders' array");
            return false;
        }
        
        // Extract and validate folder names
        std::vector<std::string> folders;
        for (const auto& folder : config["target_folders"]) {
            if (folder.is_string()) {
                std::string folder_name = folder.get<std::string>();
                if (validate_folder_name(folder_name)) {
                    folders.push_back(folder_name);
                } else {
                    Logger::instance().log(Logger::Level::Warning, 
                        "Skipping invalid folder name: " + folder_name);
                }
            }
        }
        
        if (folders.empty()) {
            Logger::instance().log(Logger::Level::Warning, 
                "No valid folder names found in configuration, using defaults");
            target_folders_ = get_default_folders();
            return false;
        }
        
        target_folders_ = folders;
        Logger::instance().log(Logger::Level::Info, 
            "Loaded " + std::to_string(folders.size()) + " folder names from configuration");
        return true;
        
    } catch (const json::exception& e) {
        Logger::instance().log(Logger::Level::Error, 
            "JSON parsing error: " + std::string(e.what()));
        return false;
    } catch (const std::exception& e) {
        Logger::instance().log(Logger::Level::Error, 
            "Error loading configuration: " + std::string(e.what()));
        return false;
    }
}

bool ConfigurationManager::save_to_file(const std::filesystem::path& config_path) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        // Create JSON object
        json config;
        config["version"] = "1.0";
        config["target_folders"] = target_folders_;
        
        // Ensure parent directory exists
        if (config_path.has_parent_path()) {
            std::filesystem::create_directories(config_path.parent_path());
        }
        
        // Write to file with pretty formatting
        std::ofstream file(config_path);
        if (!file.is_open()) {
            Logger::instance().log(Logger::Level::Error, 
                "Failed to create configuration file: " + config_path.string());
            return false;
        }
        
        file << config.dump(4); // 4-space indentation
        file.close();
        
        Logger::instance().log(Logger::Level::Info, 
            "Saved configuration to: " + config_path.string());
        return true;
        
    } catch (const json::exception& e) {
        Logger::instance().log(Logger::Level::Error, 
            "JSON serialization error: " + std::string(e.what()));
        return false;
    } catch (const std::exception& e) {
        Logger::instance().log(Logger::Level::Error, 
            "Error saving configuration: " + std::string(e.what()));
        return false;
    }
}

std::vector<std::string> ConfigurationManager::get_default_folders() {
    return {
        "node_modules",
        "venv",
        ".venv",
        "target",
        ".gradle",
        "build",
        "dist",
        "__pycache__"
    };
}

bool ConfigurationManager::validate_folder_name(const std::string& name) const {
    if (name.empty()) {
        return false;
    }
    
    // Check for path separators
    if (name.find('/') != std::string::npos || name.find('\\') != std::string::npos) {
        return false;
    }
    
    // Check for invalid characters
    const std::string invalid_chars = "<>:\"|?*";
    if (name.find_first_of(invalid_chars) != std::string::npos) {
        return false;
    }
    
    return true;
}
