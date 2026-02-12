# Fetch Catch2
include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.0
)

FetchContent_MakeAvailable(Catch2)

# Fetch RapidCheck
FetchContent_Declare(
    rapidcheck
    GIT_REPOSITORY https://github.com/emil-e/rapidcheck.git
    GIT_TAG master
)

FetchContent_MakeAvailable(rapidcheck)

# Test helpers
add_library(test_helpers STATIC
    helpers/temp_directory.cpp
    helpers/directory_tree_generator.cpp
)
target_include_directories(test_helpers PUBLIC helpers)
target_link_libraries(test_helpers PUBLIC cleanup_core)

# Unit tests
add_executable(unit_tests
    unit/test_logger.cpp
    unit/test_safety_validator.cpp
    unit/test_config.cpp
    unit/test_scanner.cpp
    unit/test_deleter.cpp
    unit/test_cli_parser.cpp
    unit/test_path_utils.cpp
)
target_link_libraries(unit_tests PRIVATE 
    cleanup_core
    cleanup_cli_lib
    test_helpers
    Catch2::Catch2WithMain
)

# Property tests
add_executable(property_tests
    property/test_properties_scanning.cpp
    property/test_properties_deletion.cpp
    property/test_properties_config.cpp
    property/test_properties_safety.cpp
)
target_link_libraries(property_tests PRIVATE 
    cleanup_core 
    test_helpers
    Catch2::Catch2WithMain
    rapidcheck
)

# Integration tests
add_executable(integration_tests
    integration/test_gui_workflow.cpp
    integration/test_cli_workflow.cpp
    integration/test_controller_workflow.cpp
)
target_link_libraries(integration_tests PRIVATE 
    cleanup_core
    cleanup_cli_lib
    test_helpers
    Catch2::Catch2WithMain
)

# Register tests with CTest
include(CTest)
include(Catch)
catch_discover_tests(unit_tests)
catch_discover_tests(property_tests)
catch_discover_tests(integration_tests)
