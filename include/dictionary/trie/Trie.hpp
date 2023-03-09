#ifndef CSD_CSD_TRIE_HPP
#define CSD_CSD_TRIE_HPP

#define TRIE 7

#include <string>
#include <vector>

#include <dictionary/trie/DataTypes.hpp>
#include <dictionary/trie/TermStringIterator.hpp>

namespace csd {

  class DataManager;

  struct TrieStats {
    std::size_t numLabelBytes; // the sum of lengths of edge labels
    std::size_t rawDataBytes; // the sum of lengths of distinct input strings
    std::size_t numLeaves;
    std::size_t numInternalNodes;
    std::size_t numEdges;
  } __attribute__((aligned(64)));

  using TriePath = std::vector<std::pair<std::size_t, Edge* const>>;

  class Trie {
  public:
    Trie();
    ~Trie();

    [[nodiscard]] auto string_to_id(const std::string& str) const -> std::size_t;
    auto id_to_string(const std::size_t& id) const -> const std::string;

    auto save(std::ostream& fp) -> void;
    auto load(unsigned char* ptr) -> void;

    auto print(std::size_t id = 0) const -> void;

    auto insert(const std::string& rdfTerm, const std::size_t& occurences = 1) -> std::pair<std::size_t, bool>;
    auto remove(const std::size_t& id, const std::size_t& occurrences = 1) -> bool;

    [[nodiscard]] auto suggestions(const std::string& prefix) const -> TermStringIterator;

    [[nodiscard]] auto getStats() const -> const TrieStats* const;
    [[nodiscard]] auto getData() const -> const DataManager* const;

    auto get_path(const std::size_t& exposedId) const -> TriePath;

    auto compare(
      const std::size_t& exposedId1,
      const std::size_t& exposedId2,
      const TriePath& path1,
      const TriePath& path2,
      const Trie* const otherCsdTrie) const -> int;
    auto compare(
      const std::size_t& exposedId1,
      const std::size_t& exposedId2) const -> int;

    void addOccurrences(const std::size_t& id, const std::size_t& occurences);

  private:
    DataManager* const m_data;
  };

}

#endif
