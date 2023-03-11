#include <filesystem>

#include <catch2/catch_all.hpp>

#include <DLDI.hpp>

TEST_CASE("Creating DLDIs from plain-text linked data") {
    auto filepath = GENERATE("foo");
    REQUIRE("foo" == filepath);
    REQUIRE(std::string{"foo"}.size()==3);
    // const auto dldi_path{std::filesystem::path{filepath}};
    // dldi::DLDI dldi{dldi_path};
}