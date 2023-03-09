#include "DataManager.hpp"
#include "utils.hpp"

namespace csd {

  auto DataManager::add_internalNode(const std::size_t& inEdgeId) -> std::size_t {
    possibly_realloc(&m_buffers.internals);
    const auto bufferIndex{m_buffers.internals.length++};
    m_buffers.internals.buf[bufferIndex] = {
      .inEdge = inEdgeId,
      .outEdgesOffset = 0, // overwritten on save
      .numOutEdges = 0,
    };
    m_stats.numInternalNodes++;
    return m_mmapPointers.internals.length + bufferIndex;
  }
  auto DataManager::remove_internalNode(const std::size_t& nodeId) -> void {
    auto* n{get_internalNode(nodeId, true)};
    n->numOutEdges = 0;
    m_stats.numInternalNodes--;
    m_numInternalNodeDeletions++;
  }

  auto DataManager::get_internalNode(const std::size_t& nodeId, bool tolerateNoOutEdges) const -> InternalNode* const {
    auto* n{get_item<struct InternalNode>(&m_mmapPointers.internals, &m_buffers.internals, nodeId)};
    if (!tolerateNoOutEdges && n->numOutEdges == 0) {
      throw std::runtime_error("Tried to get deleted internal node");
    }
    return n;
  }
  auto DataManager::internalNode_exists(const std::size_t& node_id) const -> bool {
    auto* n{get_item<struct InternalNode>(&m_mmapPointers.internals, &m_buffers.internals, node_id)};
    return n->numOutEdges > 0;
  }
}
