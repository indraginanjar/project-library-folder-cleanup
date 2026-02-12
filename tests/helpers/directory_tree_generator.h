#pragma once

#include <vector>
#include <string>
#include <filesystem>

struct DirectoryNode {
    std::string name;
    bool is_directory;
    std::vector<DirectoryNode> children;
};

class DirectoryTreeGenerator {
public:
    static DirectoryNode generate_random_tree(int max_depth, int max_children);
    static void create_tree(const std::filesystem::path& base, const DirectoryNode& node);
};
