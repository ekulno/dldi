#include <filesystem>

#include <catch2/catch_all.hpp>

#include <DLDI.hpp>

TEST_CASE("Creating DLDIs from plain-text linked data") {
    std::string filepath = GENERATE("add-1.ttl", "add-2.ttl");
    const std::filesystem::path ptld_path{"data/" + filepath};
    const std::filesystem::path dldi_path{"tmp-dldi-"+filepath};
    dldi::DLDI::from_ptld(ptld_path, dldi_path, "https://example.org/");
    dldi::DLDI{dldi_path};
}

TEST_CASE("Should throw when opening DLDI over non-dir") {
    const std::filesystem::path dldi_path{"not-a-dir"};
    REQUIRE_THROWS_WITH(dldi::DLDI{dldi_path}, "Not a directory: "+dldi_path.string());
}