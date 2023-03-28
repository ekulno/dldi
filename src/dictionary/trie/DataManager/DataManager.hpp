#ifndef CSD_DataManager_HPP
#define CSD_DataManager_HPP

#include <cstddef>
#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

#include <dictionary/trie/DataTypes.hpp>
#include <dictionary/trie/Trie.hpp>

namespace csd {

  struct Hole {
    std::size_t start;
    std::size_t size;
    // hacky field added late
    std::size_t cumulative;
  };
  template <class T>
  struct TrieBuffer {
    T* buf;
    std::size_t capacity{};
    std::size_t length{};
  };

  template <class T>
  struct TypedMmapPointer {
    T* ptr;
    std::size_t length;
  };

  struct TrieBuffers {
    TrieBuffer<LeafNode> leaves;
    TrieBuffer<InternalNode> internals;
    TrieBuffer<Edge> edges;
    TrieBuffer<unsigned char*> labels;
  };

  struct MmapPointers {
    // pointers for mem mapped representation
    struct TypedMmapPointer<LeafNode> leaves;
    struct TypedMmapPointer<InternalNode> internals;
    struct TypedMmapPointer<Edge> edges;
    struct TypedMmapPointer<unsigned char> labels;
    struct TypedMmapPointer<std::size_t> outEdgeIds;
  };

  class DataManager {
  public:
    DataManager();
    ~DataManager();

    auto save(std::ostream& fp) -> void;
    auto load(unsigned char* ptr) -> void;

    // Edges

    auto add_edge(const unsigned char* const rdfTerm, const std::size_t& from, const std::size_t& until, bool outNodeIsLeaf, const std::size_t& inNodeId, const std::size_t& outNodeId) -> std::size_t;
    auto remove_edge(const std::size_t& edgeId) -> void;
    [[nodiscard]] auto get_edge(const std::size_t& edgeId, bool dontThrowOnNotFound = false) const -> Edge* const;
    [[nodiscard]] auto edge_exists(const std::size_t& edgeId) const -> bool;
    auto edge_to_string(const std::size_t& edge_id) const -> std::string;

    // Internal nodes

    auto add_internalNode(const std::size_t& inEdgeId) -> std::size_t;
    auto remove_internalNode(const std::size_t& nodeId) -> void;
    [[nodiscard]] auto get_internalNode(const std::size_t& nodeId, bool tolerateNoOutEdges = false) const -> InternalNode* const;
    [[nodiscard]] auto internalNode_exists(const std::size_t& nodeId) const -> bool;

    // Leaf nodes

    auto add_leafNode(const std::size_t& inEdgeId, const std::size_t& occurrences = 1) -> std::size_t;
    auto remove_leafNode(const std::size_t& nodeId) -> void;
    [[nodiscard]] auto get_leafNode(const std::size_t& nodeId, bool dontThrowOnNotFound = false) const -> LeafNode* const;
    [[nodiscard]] auto internalToExposedId(const std::size_t& internalId) const -> std::size_t;
    [[nodiscard]] auto exposedToInternalId(const std::size_t& realId) const -> std::size_t;

    // Out-edges

    auto add_outEdge(const std::size_t& nodeId, const std::size_t& edgeId) const -> void;
    auto remove_outedge(const std::size_t& nodeId, const std::size_t& edgeId) const -> void;
    [[nodiscard]] auto getNewOutEdges(const std::size_t& nodeId) const -> NewOutEdgesList;

    // Labels

    [[nodiscard]] auto get_label(const std::size_t& edgeId, const struct Edge* const e = nullptr, bool dontThrowOnNotFound = false) const -> unsigned char* const;
    auto shrink_label(const std::size_t& edgeId, const std::size_t& until) -> void;

    // Statistics

    [[nodiscard]] auto getStats() const -> const TrieStats* const;

    [[nodiscard]] auto getMmapPointers() const -> const MmapPointers* const;

  private:
    TrieStats m_stats;
    TrieBuffers m_buffers;
    MmapPointers m_mmapPointers;
    std::unique_ptr<std::unordered_map<std::size_t, NewOutEdgesList>> m_outEdgesMap;
    std::vector<csd::Hole> m_loadTimeLeafHoles;
    std::vector<csd::Hole> m_finalLeafHoles;
    std::size_t m_numNewLeafNodeDeletions;
    std::size_t m_numInternalNodeDeletions;
    bool m_leafHolesComputed;
    auto computeLeafHoles() -> void;
  };
}

#endif
