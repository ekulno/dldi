#include <stdexcept>

#include <dictionary/trie/DataTypes.hpp>

#include "DataManager.hpp"
#include "utils.hpp"

namespace csd {

  auto DataManager::add_leafNode(const std::size_t& inEdgeId, const std::size_t& occurences) -> std::size_t {
    possibly_realloc(&m_buffers.leaves);
    const auto bufferIndex{m_buffers.leaves.length++};
    m_buffers.leaves.buf[bufferIndex] = {
      .inEdge = inEdgeId,
      .occurences = occurences};
    m_stats.numLeaves++;
    return m_mmapPointers.leaves.length + bufferIndex;
  }

  auto DataManager::remove_leafNode(const std::size_t& nodeId) -> void {
    if (nodeId >= m_mmapPointers.leaves.length) {
      /**
       * @brief We forbid removing statements (and by consequence, also terms) 
       * that aren't already present in the DLDI. 
       * Therefore, it's only possible to leaf nodes in the mmapped area. 
       */
      throw std::runtime_error("Tried to remove term not present in original DLDI");
    }
    get_leafNode(nodeId)->occurences = 0;
    m_stats.numLeaves--;
    m_numNewLeafNodeDeletions++;
  }
  auto DataManager::get_leafNode(const std::size_t& nodeId, bool dontThrowOnNotFound) const -> LeafNode* const {
    auto* n{get_item<struct LeafNode>(&m_mmapPointers.leaves, &m_buffers.leaves, nodeId)};
    if (!dontThrowOnNotFound && n->occurences == 0) {
      throw std::runtime_error("Tried to get deleted leaf node");
    }
    return n;
  }

  auto DataManager::internalToExposedId(const std::size_t& internalId) const -> std::size_t {
    if (m_loadTimeLeafHoles.empty()) {
      return internalId + 1;
    }

    std::size_t searchMax{m_loadTimeLeafHoles.size() - 1};
    std::size_t searchMin{0};
    std::size_t i{(searchMax - searchMin) / 2};

    while (true) {
      const auto* const hole{&m_loadTimeLeafHoles.at(i)};

      if (internalId < hole->start) {
        if (i == 0) {
          return internalId + 1;
        }
        if (m_loadTimeLeafHoles.at(i - 1).start < internalId) {
          // id is between the previous hole and this one
          return internalId + m_loadTimeLeafHoles.at(i - 1).cumulative + 1;
        }
        // id is smaller, cut search space in half. go to the left.
        searchMax = i - 1;
      } else {
        if ((i == m_loadTimeLeafHoles.size() - 1) || (m_loadTimeLeafHoles.at(i + 1).start > internalId)) {
          // id is between this and the next hole
          return internalId + hole->cumulative + 1;
        }
        searchMin = i + 1;
      }
      i = searchMax - (searchMax - searchMin) / 2;
    }
  }

  auto DataManager::exposedToInternalId(const std::size_t& exposedId) const -> std::size_t {
    if (m_loadTimeLeafHoles.empty()) {
      return exposedId - 1;
    }

    std::size_t searchMax{m_loadTimeLeafHoles.size() - 1};
    std::size_t searchMin{0};
    std::size_t i{(searchMax - searchMin) / 2};

    while (true) {
      const auto* const curr{&m_loadTimeLeafHoles.at(i)};
      const auto currFirst = curr->start + 1 + ((i > 0) ? (&m_loadTimeLeafHoles.at(i - 1))->cumulative : 0);
      if (exposedId < currFirst) {
        if (i == 0) {
          // no holes apply
          return exposedId - 1;
        }
        const auto* const prev{&m_loadTimeLeafHoles.at(i - 1)};
        const auto prevLast{prev->start + prev->size - 1 + prev->cumulative + 1};
        if (exposedId > prevLast) {
          // between this and the previous hole
          return exposedId - (&m_loadTimeLeafHoles.at(i - 1))->cumulative - 1;
        }
        searchMax = i - 1;
      } else {
        if (i == m_loadTimeLeafHoles.size() - 1 || (exposedId < (&m_loadTimeLeafHoles.at(i + 1))->start + curr->cumulative + 1)) {
          return exposedId - curr->cumulative - 1;
        }
        searchMin = i + 1;
      }
      i = searchMax - (searchMax - searchMin) / 2;
    }
  }
}
