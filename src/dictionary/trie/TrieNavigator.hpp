#ifndef TRIE_NAVIGATOR_HPP
#define TRIE_NAVIGATOR_HPP

#include <dictionary/trie/DataTypes.hpp>
#include <dictionary/trie/OutEdgeIterator.hpp>

namespace csd {

  class TrieNavigator {
  public:
    TrieNavigator(const DataManager* const data, const std::size_t& startingNodeId = 0);
    ~TrieNavigator();
    auto goRight() -> void;
    auto goDown() -> void;
    auto mayGoRight() -> bool;
    [[nodiscard]] auto mayGoDown() const -> bool;
    [[nodiscard]] auto edge() const -> Edge* const;
    [[nodiscard]] auto label() const -> unsigned char* const;
    [[nodiscard]] auto leaf(bool throwOnDeleted = true) const -> LeafNode* const;
    [[nodiscard]] auto outNode() -> InternalNode* const;
    [[nodiscard]] auto inNode() -> InternalNode* const;
    [[nodiscard]] auto edgeId() const -> std::size_t;
    auto refreshEdge() -> void;

  private:
    const DataManager* const m_data;
    InternalNode* m_upper;
    InternalNode* m_lower;
    Edge* m_edge;
    std::size_t m_edgeId;
    OutEdgeIterator* m_it;
  };
}
#endif
