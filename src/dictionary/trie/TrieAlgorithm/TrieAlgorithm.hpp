#ifndef CSD_TRIE_ALGORITHM_HPP
#define CSD_TRIE_ALGORITHM_HPP

#include <cstddef>

#include "../DataManager/DataManager.hpp"

namespace csd {

  struct StringNotFoundException : public std::exception {
    [[nodiscard]] auto what() const noexcept -> const char* override {
      return "String not found";
    }
  };

  class TrieAlgorithm {
  public:
    // read-only operations

    static auto id_to_string(const DataManager* const data, const std::size_t& id, bool dontThrowOnNotFound = false) -> std::string;
    static auto extract_path(const DataManager* const data, const std::size_t& id, bool dontThrowOnNotFound = false) -> TriePath;
    static auto string_to_id(const DataManager* const data, const std::string& rdfTerm) -> std::pair<std::size_t, std::size_t>;
    static auto get_scope(const DataManager* const data, const std::string& prefix) -> std::tuple<std::size_t, bool, bool>;
    static auto compile_path_label(const DataManager* const data, const TriePath& path, bool dontThrowOnNotFound) -> std::string;

    // update operations

    static auto insert(DataManager* data, const std::string& rdfTerm, const std::size_t& occurences) -> std::pair<std::size_t, bool>;
    static auto remove(DataManager* data, const std::size_t& id, const std::size_t& occurences = 1) -> bool;
  };
}

#endif
