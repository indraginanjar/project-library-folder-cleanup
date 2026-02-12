#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <mutex>

class ConfigurationManager {
public:
    static ConfigurationManager& instance();
    
    std::vector<std::string> get_target_folders() const;
    void set_target_folders(const std::vector<std::string>& folders);
    
    bool load_from_file(const std::filesystem::path& config_path);
    bool save_to_file(const std::filesystem::path& config_path);
    
    static std::vector<std::string> get_default_folders();

private:
    ConfigurationManager();
    ConfigurationManager(const ConfigurationManager&) = delete;
    ConfigurationManager& operator=(const ConfigurationManager&) = delete;

    std::vector<std::string> target_folders_;
    mutable std::mutex mutex_;
    
    bool validate_folder_name(const std::string& name) const;
};
