#include <catch2/catch_all.hpp>
#include <filesystem>
#include <vector>

#include <DLDI.hpp>
#include <DLDI_Composer.hpp>

// NB: avoid file path conflicts across tests.
// Tests are run in parallel.
// Use temporary_directory when necessary.

inline auto temporary_directory(const std::filesystem::path& base) -> std::filesystem::path {
  const std::string template_local_name(base.string() + ".XXXXXX");
  const std::filesystem::path template_absolute_path{std::filesystem::temp_directory_path() / template_local_name};
  auto* const mutable_absolute_path{const_cast<char*>(template_absolute_path.c_str())};
  const auto mkdtemp_result{mkdtemp(mutable_absolute_path)};
  if (mkdtemp_result == nullptr) {
    const std::string error_message{std::strerror(errno)};
    throw std::runtime_error("Failed to create temporary directory: " + error_message);
  }
  return std::filesystem::path{mkdtemp_result};
}

TEST_CASE("Creating DLDIs from plain-text linked data") {
  const auto tmpdir{temporary_directory("create")};

  std::string filepath = GENERATE("add-1.ttl", "add-2.ttl", "rem-1.ttl", "rem-2.ttl");
  const std::filesystem::path ptld_path{"data/" + filepath};
  const std::filesystem::path dldi_path{tmpdir / ("dldi-" + filepath)};
  dldi::DLDI::from_ptld(ptld_path, dldi_path, "https://example.org/");
  dldi::DLDI{dldi_path};
}

TEST_CASE("Should throw when opening DLDI over non-dir") {
  const std::filesystem::path dldi_path{"not-a-dir"};
  REQUIRE_THROWS_WITH(dldi::DLDI{dldi_path}, "Not a directory: " + dldi_path.string());
}

TEST_CASE("Should compose from files to add and remove") {
  const auto tmpdir{temporary_directory("merge")};

  dldi::DLDI::from_ptld("data/add-1.ttl", tmpdir / "add-1.dldi", "https://example.org/");
  dldi::DLDI::from_ptld("data/add-2.ttl", tmpdir / "add-2.dldi", "https://example.org/");
  dldi::DLDI::from_ptld("data/rem-1.ttl", tmpdir / "rem-1.dldi", "https://example.org/");
  dldi::DLDI::from_ptld("data/rem-2.ttl", tmpdir / "rem-2.dldi", "https://example.org/");
  dldi::DLDI_Composer::compose(std::vector<std::filesystem::path>{tmpdir / "add-1.dldi", tmpdir / "add-2.dldi"},
                      std::vector<std::filesystem::path>{tmpdir / "rem-1.dldi", tmpdir / "rem-2.dldi"},
                      tmpdir / "merged.dldi");
}

TEST_CASE("Should handle terms which are strict prefixes of another") {
  const auto tmpdir{temporary_directory("prefixes")};
  SECTION("case 1") {
    dldi::DLDI::from_ptld("data/prefixed-1.nt", tmpdir / "prefixed-1.dldi", "https://example.org/");
  }
  SECTION("case 2") {
    dldi::DLDI::from_ptld("data/prefixed-2.nt", tmpdir / "prefixed-2.dldi", "https://example.org/");
  }
}

TEST_CASE("Should query unmerged dldi") {

  const auto tmpdir{temporary_directory("query")};

  dldi::DLDI::from_ptld("data/combined.ttl", tmpdir / "combined.dldi", "https://example.org/");

  dldi::DLDI dldi{tmpdir / "combined.dldi"};

  SECTION("term query 100"){
    dldi.ensure_loaded(dldi::TripleTermPosition::subject);
    auto it{dldi.terms("", true, false, false)};
    auto num_results{0};
    while (it->has_next()) {
      const auto term{it->next()};
      ++num_results;
    }
    REQUIRE(num_results == 3);
  }
  SECTION("term query 010"){
    dldi.ensure_loaded(dldi::TripleTermPosition::predicate);
    auto it{dldi.terms("", false, true, false)};
    auto num_results{0};
    while (it->has_next()) {
      const auto term{it->next()};
      ++num_results;
    }
    REQUIRE(num_results == 2);
  }
  SECTION("term query 110"){
    dldi.ensure_loaded(dldi::TripleTermPosition::subject);
    dldi.ensure_loaded(dldi::TripleTermPosition::predicate);
    auto it{dldi.terms("", true, true, false)};
    auto num_results{0};
    while (it->has_next()) {
      const auto term{it->next()};
      ++num_results;
    }
    REQUIRE(num_results == 5);
  }
  SECTION("term query 001"){
    dldi.ensure_loaded(dldi::TripleTermPosition::object);
    auto it{dldi.terms("", false, false, true)};
    auto num_results{0};
    while (it->has_next()) {
      const auto term{it->next()};
      ++num_results;
    }
    REQUIRE(num_results == 5);
  }
  SECTION("term query 101"){
    dldi.ensure_loaded(dldi::TripleTermPosition::subject);
    dldi.ensure_loaded(dldi::TripleTermPosition::object);
    auto it{dldi.terms("", true, false, true)};
    auto num_results{0};
    while (it->has_next()) {
      const auto term{it->next()};
      ++num_results;
    }
    REQUIRE(num_results == 6);
  }
  SECTION("term query 111"){
    dldi.ensure_loaded(dldi::TripleTermPosition::subject);
    dldi.ensure_loaded(dldi::TripleTermPosition::predicate);
    dldi.ensure_loaded(dldi::TripleTermPosition::object);
    auto it{dldi.terms("", true, true, true)};
    auto num_results{0};
    while (it->has_next()) {
      const auto term{it->next()};
      ++num_results;
    }
    REQUIRE(num_results == 8);
  }
  SECTION("term query 100 with prefix '\"2'"){
    dldi.ensure_loaded(dldi::TripleTermPosition::subject);
    dldi.ensure_loaded(dldi::TripleTermPosition::object);
    auto it{dldi.terms("\"2", true, false, true)};
    auto num_results{0};
    while (it->has_next()) {
      const auto term{it->next()};
      ++num_results;
    }
    REQUIRE(num_results == 2);
  }
  SECTION("triple-pattern query 000"){
    const dldi::TriplePattern pattern{0, 0, 0};
    dldi.prepare_for_query(pattern);
    auto it{dldi.search(pattern)};
    auto num_results{0};
    while (it->has_next()) {
      const auto triple{it->read()};
      ++num_results;
      it->proceed();
    }
    REQUIRE(num_results == 6);
  }
  SECTION("triple-pattern query 110"){
    dldi.ensure_loaded(dldi::TripleTermPosition::subject);
    dldi.ensure_loaded(dldi::TripleTermPosition::predicate);
    const dldi::TriplePattern pattern{
      dldi.string_to_id("http://example.com/t1", dldi::TripleTermPosition::subject),
      dldi.string_to_id("http://example.com/pred1", dldi::TripleTermPosition::predicate),
      0};
    dldi.prepare_for_query(pattern);
    auto it{dldi.search(pattern)};
    auto num_results{0};
    while (it->has_next()) {
      const auto triple{it->read()};
      ++num_results;
      it->proceed();
    }
    REQUIRE(num_results == 2);
  }
  SECTION("triple-pattern query 010"){
    dldi.ensure_loaded(dldi::TripleTermPosition::predicate);
    const dldi::TriplePattern pattern{0, dldi.string_to_id("http://example.com/pred2", dldi::TripleTermPosition::predicate), 0};
    dldi.prepare_for_query(pattern);
    auto it{dldi.search(pattern)};
    auto num_results{0};
    while (it->has_next()) {
      const auto triple{it->read()};
      ++num_results;
      it->proceed();
    }
    REQUIRE(num_results == 1);
  }
}

TEST_CASE("Should query merged dldi") {
  const auto tmpdir{temporary_directory("query")};

  dldi::DLDI::from_ptld("data/add-1.ttl", tmpdir / "add-1.dldi", "https://example.org/");
  dldi::DLDI::from_ptld("data/add-2.ttl", tmpdir / "add-2.dldi", "https://example.org/");
  dldi::DLDI::from_ptld("data/rem-1.ttl", tmpdir / "rem-1.dldi", "https://example.org/");
  dldi::DLDI::from_ptld("data/rem-2.ttl", tmpdir / "rem-2.dldi", "https://example.org/");
  dldi::DLDI_Composer::compose(std::vector<std::filesystem::path>{tmpdir / "add-1.dldi", tmpdir / "add-2.dldi"},
                      std::vector<std::filesystem::path>{tmpdir / "rem-1.dldi", tmpdir / "rem-2.dldi"},
                      tmpdir / "merged.dldi");

  dldi::DLDI dldi{tmpdir / "merged.dldi"};

  SECTION("term query 100"){
    dldi.ensure_loaded(dldi::TripleTermPosition::subject);
    auto it{dldi.terms("", true, false, false)};
    auto num_results{0};
    while (it->has_next()) {
      const auto term{it->next()};
      ++num_results;
    }
    REQUIRE(num_results == 3);
  }
  SECTION("term query 010"){
    dldi.ensure_loaded(dldi::TripleTermPosition::predicate);
    auto it{dldi.terms("", false, true, false)};
    auto num_results{0};
    while (it->has_next()) {
      const auto term{it->next()};
      ++num_results;
    }
    REQUIRE(num_results == 2);
  }
  SECTION("term query 110"){
    dldi.ensure_loaded(dldi::TripleTermPosition::subject);
    dldi.ensure_loaded(dldi::TripleTermPosition::predicate);
    auto it{dldi.terms("", true, true, false)};
    auto num_results{0};
    while (it->has_next()) {
      const auto term{it->next()};
      ++num_results;
    }
    REQUIRE(num_results == 5);
  }
  SECTION("term query 001"){
    dldi.ensure_loaded(dldi::TripleTermPosition::object);
    auto it{dldi.terms("", false, false, true)};
    auto num_results{0};
    while (it->has_next()) {
      const auto term{it->next()};
      ++num_results;
    }
    REQUIRE(num_results == 5);
  }
  SECTION("term query 101"){
    dldi.ensure_loaded(dldi::TripleTermPosition::subject);
    dldi.ensure_loaded(dldi::TripleTermPosition::object);
    auto it{dldi.terms("", true, false, true)};
    auto num_results{0};
    while (it->has_next()) {
      const auto term{it->next()};
      ++num_results;
    }
    REQUIRE(num_results == 6);
  }
  SECTION("term query 111"){
    dldi.ensure_loaded(dldi::TripleTermPosition::subject);
    dldi.ensure_loaded(dldi::TripleTermPosition::predicate);
    dldi.ensure_loaded(dldi::TripleTermPosition::object);
    auto it{dldi.terms("", true, true, true)};
    auto num_results{0};
    while (it->has_next()) {
      const auto term{it->next()};
      ++num_results;
    }
    REQUIRE(num_results == 8);
  }
  SECTION("term query 100 with prefix '\"2'"){
    dldi.ensure_loaded(dldi::TripleTermPosition::subject);
    dldi.ensure_loaded(dldi::TripleTermPosition::object);
    auto it{dldi.terms("\"2", true, false, true)};
    auto num_results{0};
    while (it->has_next()) {
      const auto term{it->next()};
      ++num_results;
    }
    REQUIRE(num_results == 2);
  }
  SECTION("triple-pattern query 000"){
    const dldi::TriplePattern pattern{0, 0, 0};
    dldi.prepare_for_query(pattern);
    auto it{dldi.search(pattern)};
    auto num_results{0};
    while (it->has_next()) {
      const auto triple{it->read()};
      ++num_results;
      it->proceed();
    }
    REQUIRE(num_results == 6);
  }
  SECTION("triple-pattern query 110"){
    dldi.ensure_loaded(dldi::TripleTermPosition::subject);
    dldi.ensure_loaded(dldi::TripleTermPosition::predicate);
    const dldi::TriplePattern pattern{
      dldi.string_to_id("http://example.com/t1", dldi::TripleTermPosition::subject),
      dldi.string_to_id("http://example.com/pred1", dldi::TripleTermPosition::predicate),
      0};
    dldi.prepare_for_query(pattern);
    auto it{dldi.search(pattern)};
    auto num_results{0};
    while (it->has_next()) {
      const auto triple{it->read()};
      ++num_results;
      it->proceed();
    }
    REQUIRE(num_results == 2);
  }
  SECTION("triple-pattern query 010"){
    dldi.ensure_loaded(dldi::TripleTermPosition::predicate);
    const dldi::TriplePattern pattern{0, dldi.string_to_id("http://example.com/pred2", dldi::TripleTermPosition::predicate), 0};
    dldi.prepare_for_query(pattern);
    auto it{dldi.search(pattern)};
    auto num_results{0};
    while (it->has_next()) {
      const auto triple{it->read()};
      ++num_results;
      it->proceed();
    }
    REQUIRE(num_results == 1);
  }
}
