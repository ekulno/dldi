#include <cstddef>

#include <dictionary/trie/DataTypes.hpp>

#include "DataManager.hpp"

#define INITIAL_CAPCITY 1

namespace csd {

  auto DataManager::load(unsigned char* ptr) -> void {
    m_mmapPointers.leaves.length = *reinterpret_cast<const std::size_t* const>(ptr);
    ptr += sizeof(std::size_t);

    m_mmapPointers.internals.length = *reinterpret_cast<const std::size_t* const>(ptr);
    ptr += sizeof(std::size_t);

    m_mmapPointers.edges.length = *reinterpret_cast<const std::size_t* const>(ptr);
    m_mmapPointers.outEdgeIds.length = m_mmapPointers.edges.length;
    ptr += sizeof(std::size_t);

    m_mmapPointers.labels.length = *reinterpret_cast<const std::size_t* const>(ptr);
    ptr += sizeof(std::size_t);

    const auto num_leaf_holes = *reinterpret_cast<const std::size_t* const>(ptr);
    ptr += sizeof(std::size_t);

    m_stats.numLeaves = m_mmapPointers.leaves.length;
    m_stats.numInternalNodes = m_mmapPointers.internals.length;
    m_stats.numEdges = m_mmapPointers.edges.length;
    m_stats.numLabelBytes = m_mmapPointers.labels.length;

    m_mmapPointers.labels.ptr = ptr;
    ptr += m_stats.numLabelBytes;

    m_mmapPointers.edges.ptr = reinterpret_cast<Edge* const>(ptr);
    ptr += sizeof(struct Edge) * m_stats.numEdges;

    m_mmapPointers.leaves.ptr = reinterpret_cast<LeafNode* const>(ptr);
    ptr += sizeof(struct LeafNode) * m_stats.numLeaves;

    auto* nodeHolesPtr{ptr};
    ptr += sizeof(struct Hole) * num_leaf_holes;

    m_mmapPointers.outEdgeIds.ptr = reinterpret_cast<std::size_t* const>(ptr);
    ptr += m_stats.numEdges * sizeof(std::size_t);

    m_mmapPointers.internals.ptr = reinterpret_cast<InternalNode* const>(ptr);
    ptr += sizeof(struct InternalNode) * m_stats.numInternalNodes;

    m_loadTimeLeafHoles.reserve(num_leaf_holes);
    for (std::size_t i = 0; i < num_leaf_holes; i++) {
      const auto* const h = reinterpret_cast<const struct Hole*>(nodeHolesPtr + i * sizeof(std::size_t) * 3);
      struct Hole newHole {
        .start = h->start,
        .size = h->size,
        .cumulative = h->cumulative
      };
      m_loadTimeLeafHoles.push_back(newHole);
    }
  }
}
