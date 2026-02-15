# Core library sources
set(CORE_SOURCES
    core/Logger.cpp
    core/SafetyValidator.cpp
    core/ConfigurationManager.cpp
    core/FileSystemScanner.cpp
    core/FolderDeleter.cpp
    core/ApplicationController.cpp
    core/PathUtils.cpp
)

set(CORE_HEADERS
    ${CMAKE_SOURCE_DIR}/include/Logger.h
    ${CMAKE_SOURCE_DIR}/include/SafetyValidator.h
    ${CMAKE_SOURCE_DIR}/include/ConfigurationManager.h
    ${CMAKE_SOURCE_DIR}/include/FileSystemScanner.h
    ${CMAKE_SOURCE_DIR}/include/FolderDeleter.h
    ${CMAKE_SOURCE_DIR}/include/ApplicationController.h
    ${CMAKE_SOURCE_DIR}/include/PathUtils.h
)

# Core library
add_library(cleanup_core STATIC ${CORE_SOURCES} ${CORE_HEADERS})
target_include_directories(cleanup_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(cleanup_core PUBLIC nlohmann_json::nlohmann_json)

# CLI library (for testing)
set(CLI_SOURCES
    cli/CLIApplication.cpp
)

set(CLI_HEADERS
    ${CMAKE_SOURCE_DIR}/include/CLIApplication.h
)

add_library(cleanup_cli_lib STATIC ${CLI_SOURCES} ${CLI_HEADERS})
target_include_directories(cleanup_cli_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(cleanup_cli_lib PUBLIC cleanup_core)

# CLI application
if(BUILD_CLI)
    add_executable(plf_cleanup_cli 
        cli/main_cli.cpp
    )
    target_link_libraries(plf_cleanup_cli PRIVATE cleanup_cli_lib)
endif()

# GUI application
if(BUILD_GUI)
    set(GUI_SOURCES
        gui/MainWindow.cpp
        gui/ConfigDialog.cpp
        gui/ScanWorker.cpp
        gui/DeleteWorker.cpp
        gui/main_gui.cpp
    )
    
    set(GUI_HEADERS
        ${CMAKE_SOURCE_DIR}/include/MainWindow.h
        ${CMAKE_SOURCE_DIR}/include/ConfigDialog.h
        ${CMAKE_SOURCE_DIR}/include/ScanWorker.h
        ${CMAKE_SOURCE_DIR}/include/DeleteWorker.h
    )
    
    set(GUI_UI
        gui/MainWindow.ui
        gui/ConfigDialog.ui
    )
    
    add_executable(plf_cleanup_gui ${GUI_SOURCES} ${GUI_HEADERS} ${GUI_UI})
    target_link_libraries(plf_cleanup_gui PRIVATE cleanup_core Qt6::Core Qt6::Widgets)
    target_include_directories(plf_cleanup_gui PRIVATE ${CMAKE_SOURCE_DIR}/include)
endif()
