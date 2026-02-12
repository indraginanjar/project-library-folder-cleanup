#include "directory_tree_generator.h"
#include <random>
#include <fstream>

DirectoryNode DirectoryTreeGenerator::generate_random_tree(int max_depth, int max_children) {
    // Placeholder implementation
    DirectoryNode root;
    root.name = "root";
    root.is_directory = true;
    return root;
}

void DirectoryTreeGenerator::create_tree(const std::filesystem::path& base, const DirectoryNode& node) {
    if (node.is_directory) {
        std::filesystem::create_directories(base / node.name);
        
        for (const auto& child : node.children) {
            create_tree(base / node.name, child);
        }
    } else {
        std::ofstream file(base / node.name);
        file << "test content";
    }
}
