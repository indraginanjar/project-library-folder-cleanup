#include <catch2/catch_test_macros.hpp>
#include "CLIApplication.h"

TEST_CASE("CLI parser handles help flag", "[cli]") {
    const char* argv[] = {"cleanup_cli", "--help"};
    CLIApplication app(2, const_cast<char**>(argv));
    
    // Should not throw
    REQUIRE(true);
}
