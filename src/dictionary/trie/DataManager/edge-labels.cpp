#include "DataManager.hpp"

namespace csd {

  auto DataManager::get_label(const std::size_t& edgeId, const struct Edge* const edge, bool dontThrowOnDeletedEdge) const -> unsigned char* const {
    if (edgeId < m_mmapPointers.edges.length) {
      const auto* const edge_{(edge == nullptr) ? get_edge(edgeId, dontThrowOnDeletedEdge) : edge};
      return m_mmapPointers.labels.ptr + edge_->labelOffset;
    }
    return m_buffers.labels.buf[edgeId - m_mmapPointers.edges.length];
  }

  auto DataManager::shrink_label(const std::size_t& edgeId, const std::size_t& newLength) -> void {
    struct Edge* const e{get_edge(edgeId)};
    m_stats.numLabelBytes -= (e->labelLength - newLength);
    e->labelLength = newLength;
  }

}
