#include "temp_directory.h"
#include <fstream>
#include <random>

TempDirectory::TempDirectory() {
    auto temp = std::filesystem::temp_directory_path();
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(100000, 999999);
    
    path_ = temp / ("test_cleanup_" + std::to_string(dis(gen)));
    std::filesystem::create_directories(path_);
}

TempDirectory::~TempDirectory() {
    if (std::filesystem::exists(path_)) {
        std::filesystem::remove_all(path_);
    }
}

void TempDirectory::create_file(const std::string& relative_path, const std::string& content) {
    auto file_path = path_ / relative_path;
    std::filesystem::create_directories(file_path.parent_path());
    
    std::ofstream file(file_path);
    file << content;
}

void TempDirectory::create_directory(const std::string& relative_path) {
    auto dir_path = path_ / relative_path;
    std::filesystem::create_directories(dir_path);
}
