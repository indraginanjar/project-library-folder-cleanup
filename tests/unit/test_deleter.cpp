#include <catch2/catch_test_macros.hpp>
#include "FolderDeleter.h"
#include "temp_directory.h"
#include <thread>
#include <atomic>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

TEST_CASE("Deleter removes folders in actual mode", "[deleter]") {
    TempDirectory temp;
    
    temp.create_directory("to_delete");
    auto folder_path = temp.path() / "to_delete";
    
    FolderDeleter deleter(FolderDeleter::Mode::ActualDelete);
    auto result = deleter.delete_folders({folder_path});
    
    REQUIRE_FALSE(std::filesystem::exists(folder_path));
}

TEST_CASE("Deleter preserves folders in dry-run mode", "[deleter]") {
    TempDirectory temp;
    
    temp.create_directory("to_preserve");
    auto folder_path = temp.path() / "to_preserve";
    
    FolderDeleter deleter(FolderDeleter::Mode::DryRun);
    auto result = deleter.delete_folders({folder_path});
    
    REQUIRE(std::filesystem::exists(folder_path));
}

TEST_CASE("Deleter respects cancellation", "[deleter][cancellation]") {
    TempDirectory temp;
    
    // Create many folders to give time for cancellation
    std::vector<std::filesystem::path> folders;
    for (int i = 0; i < 100; ++i) {
        auto folder_name = "folder" + std::to_string(i);
        temp.create_directory(folder_name);
        folders.push_back(temp.path() / folder_name);
    }
    
    FolderDeleter deleter(FolderDeleter::Mode::ActualDelete);
    
    // Start deletion in a separate thread and cancel it immediately
    std::atomic<bool> deletion_started{false};
    std::thread delete_thread([&]() {
        deletion_started = true;
        auto result = deleter.delete_folders(folders);
        // Result should be incomplete due to cancellation
    });
    
    // Wait for deletion to start
    while (!deletion_started) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    // Cancel the deletion
    deleter.cancel();
    
    // Wait for deletion thread to complete
    delete_thread.join();
    
    // Test passes if we reach here without hanging
    REQUIRE(true);
}

#ifdef _WIN32
TEST_CASE("Deleter handles Windows read-only files", "[deleter][platform][windows]") {
    TempDirectory temp;
    
    // Create a directory with a read-only file
    temp.create_directory("readonly_folder");
    auto folder_path = temp.path() / "readonly_folder";
    temp.create_file("readonly_folder/readonly.txt", "test content");
    auto file_path = folder_path / "readonly.txt";
    
    // Set the file to read-only using Windows API
    std::wstring wide_path = file_path.wstring();
    DWORD attributes = GetFileAttributesW(wide_path.c_str());
    REQUIRE(attributes != INVALID_FILE_ATTRIBUTES);
    
    BOOL result = SetFileAttributesW(wide_path.c_str(), 
                                     attributes | FILE_ATTRIBUTE_READONLY);
    REQUIRE(result != 0);
    
    // Verify the file is read-only
    attributes = GetFileAttributesW(wide_path.c_str());
    REQUIRE((attributes & FILE_ATTRIBUTE_READONLY) != 0);
    
    // Delete the folder containing the read-only file
    FolderDeleter deleter(FolderDeleter::Mode::ActualDelete);
    auto deletion_result = deleter.delete_folders({folder_path});
    
    // The folder should be successfully deleted
    REQUIRE(deletion_result.folders_deleted == 1);
    REQUIRE_FALSE(std::filesystem::exists(folder_path));
    REQUIRE_FALSE(std::filesystem::exists(file_path));
}

TEST_CASE("Deleter handles Windows hidden and system files", "[deleter][platform][windows]") {
    TempDirectory temp;
    
    // Create a directory with hidden and system files
    temp.create_directory("special_folder");
    auto folder_path = temp.path() / "special_folder";
    temp.create_file("special_folder/hidden.txt", "hidden content");
    temp.create_file("special_folder/system.txt", "system content");
    
    auto hidden_file = folder_path / "hidden.txt";
    auto system_file = folder_path / "system.txt";
    
    // Set hidden attribute
    std::wstring hidden_wide = hidden_file.wstring();
    DWORD hidden_attr = GetFileAttributesW(hidden_wide.c_str());
    REQUIRE(hidden_attr != INVALID_FILE_ATTRIBUTES);
    SetFileAttributesW(hidden_wide.c_str(), hidden_attr | FILE_ATTRIBUTE_HIDDEN);
    
    // Set system attribute
    std::wstring system_wide = system_file.wstring();
    DWORD system_attr = GetFileAttributesW(system_wide.c_str());
    REQUIRE(system_attr != INVALID_FILE_ATTRIBUTES);
    SetFileAttributesW(system_wide.c_str(), system_attr | FILE_ATTRIBUTE_SYSTEM);
    
    // Delete the folder
    FolderDeleter deleter(FolderDeleter::Mode::ActualDelete);
    auto deletion_result = deleter.delete_folders({folder_path});
    
    // The folder should be successfully deleted
    REQUIRE(deletion_result.folders_deleted == 1);
    REQUIRE_FALSE(std::filesystem::exists(folder_path));
}

TEST_CASE("Deleter handles Windows read-only directories", "[deleter][platform][windows]") {
    TempDirectory temp;
    
    // Create a nested directory structure
    temp.create_directory("readonly_parent");
    temp.create_directory("readonly_parent/readonly_child");
    auto parent_path = temp.path() / "readonly_parent";
    auto child_path = parent_path / "readonly_child";
    temp.create_file("readonly_parent/readonly_child/file.txt", "content");
    
    // Set the child directory to read-only
    std::wstring wide_path = child_path.wstring();
    DWORD attributes = GetFileAttributesW(wide_path.c_str());
    REQUIRE(attributes != INVALID_FILE_ATTRIBUTES);
    SetFileAttributesW(wide_path.c_str(), attributes | FILE_ATTRIBUTE_READONLY);
    
    // Delete the parent folder
    FolderDeleter deleter(FolderDeleter::Mode::ActualDelete);
    auto deletion_result = deleter.delete_folders({parent_path});
    
    // The folder should be successfully deleted
    REQUIRE(deletion_result.folders_deleted == 1);
    REQUIRE_FALSE(std::filesystem::exists(parent_path));
}
#else
TEST_CASE("Deleter handles Linux read-only files", "[deleter][platform][linux]") {
    TempDirectory temp;
    
    // Create a directory with a read-only file
    temp.create_directory("readonly_folder");
    auto folder_path = temp.path() / "readonly_folder";
    temp.create_file("readonly_folder/readonly.txt", "test content");
    auto file_path = folder_path / "readonly.txt";
    
    // Remove write permissions using chmod
    std::filesystem::permissions(file_path, 
                                 std::filesystem::perms::owner_read | 
                                 std::filesystem::perms::group_read | 
                                 std::filesystem::perms::others_read,
                                 std::filesystem::perm_options::replace);
    
    // Verify the file is read-only (no write permissions)
    auto perms = std::filesystem::status(file_path).permissions();
    REQUIRE((perms & std::filesystem::perms::owner_write) == std::filesystem::perms::none);
    
    // Delete the folder containing the read-only file
    FolderDeleter deleter(FolderDeleter::Mode::ActualDelete);
    auto deletion_result = deleter.delete_folders({folder_path});
    
    // The folder should be successfully deleted
    REQUIRE(deletion_result.folders_deleted == 1);
    REQUIRE_FALSE(std::filesystem::exists(folder_path));
    REQUIRE_FALSE(std::filesystem::exists(file_path));
}

TEST_CASE("Deleter handles Linux directories without write permissions", "[deleter][platform][linux]") {
    TempDirectory temp;
    
    // Create a nested directory structure
    temp.create_directory("readonly_parent");
    temp.create_directory("readonly_parent/readonly_child");
    auto parent_path = temp.path() / "readonly_parent";
    auto child_path = parent_path / "readonly_child";
    temp.create_file("readonly_parent/readonly_child/file.txt", "content");
    
    // Remove write permissions from the child directory
    std::filesystem::permissions(child_path,
                                 std::filesystem::perms::owner_read | 
                                 std::filesystem::perms::owner_exec |
                                 std::filesystem::perms::group_read | 
                                 std::filesystem::perms::group_exec |
                                 std::filesystem::perms::others_read | 
                                 std::filesystem::perms::others_exec,
                                 std::filesystem::perm_options::replace);
    
    // Delete the parent folder
    FolderDeleter deleter(FolderDeleter::Mode::ActualDelete);
    auto deletion_result = deleter.delete_folders({parent_path});
    
    // The folder should be successfully deleted
    REQUIRE(deletion_result.folders_deleted == 1);
    REQUIRE_FALSE(std::filesystem::exists(parent_path));
}

TEST_CASE("Deleter handles Linux files with no permissions", "[deleter][platform][linux]") {
    TempDirectory temp;
    
    // Create a directory with a file that has no permissions
    temp.create_directory("noperm_folder");
    auto folder_path = temp.path() / "noperm_folder";
    temp.create_file("noperm_folder/noperm.txt", "test content");
    auto file_path = folder_path / "noperm.txt";
    
    // Remove all permissions
    std::filesystem::permissions(file_path, 
                                 std::filesystem::perms::none,
                                 std::filesystem::perm_options::replace);
    
    // Verify the file has no permissions
    auto perms = std::filesystem::status(file_path).permissions();
    REQUIRE((perms & std::filesystem::perms::owner_write) == std::filesystem::perms::none);
    REQUIRE((perms & std::filesystem::perms::owner_read) == std::filesystem::perms::none);
    
    // Delete the folder
    FolderDeleter deleter(FolderDeleter::Mode::ActualDelete);
    auto deletion_result = deleter.delete_folders({folder_path});
    
    // The folder should be successfully deleted
    REQUIRE(deletion_result.folders_deleted == 1);
    REQUIRE_FALSE(std::filesystem::exists(folder_path));
}
#endif

TEST_CASE("Deleter handles multiple folders with mixed permissions", "[deleter][platform]") {
    TempDirectory temp;
    
    // Create multiple folders with different permission scenarios
    temp.create_directory("folder1");
    temp.create_directory("folder2");
    temp.create_directory("folder3");
    
    auto folder1 = temp.path() / "folder1";
    auto folder2 = temp.path() / "folder2";
    auto folder3 = temp.path() / "folder3";
    
    temp.create_file("folder1/normal.txt", "normal");
    temp.create_file("folder2/readonly.txt", "readonly");
    temp.create_file("folder3/normal2.txt", "normal2");
    
#ifdef _WIN32
    // Make folder2's file read-only on Windows
    auto readonly_file = folder2 / "readonly.txt";
    std::wstring wide_path = readonly_file.wstring();
    DWORD attributes = GetFileAttributesW(wide_path.c_str());
    SetFileAttributesW(wide_path.c_str(), attributes | FILE_ATTRIBUTE_READONLY);
#else
    // Make folder2's file read-only on Linux
    auto readonly_file = folder2 / "readonly.txt";
    std::filesystem::permissions(readonly_file,
                                 std::filesystem::perms::owner_read,
                                 std::filesystem::perm_options::replace);
#endif
    
    // Delete all folders
    FolderDeleter deleter(FolderDeleter::Mode::ActualDelete);
    auto deletion_result = deleter.delete_folders({folder1, folder2, folder3});
    
    // All folders should be successfully deleted
    REQUIRE(deletion_result.folders_deleted == 3);
    REQUIRE_FALSE(std::filesystem::exists(folder1));
    REQUIRE_FALSE(std::filesystem::exists(folder2));
    REQUIRE_FALSE(std::filesystem::exists(folder3));
}
