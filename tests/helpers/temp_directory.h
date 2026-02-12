#pragma once

#include <filesystem>
#include <string>

class TempDirectory {
public:
    TempDirectory();
    ~TempDirectory();
    
    const std::filesystem::path& path() const { return path_; }
    
    void create_file(const std::string& relative_path, const std::string& content = "");
    void create_directory(const std::string& relative_path);

private:
    std::filesystem::path path_;
};
