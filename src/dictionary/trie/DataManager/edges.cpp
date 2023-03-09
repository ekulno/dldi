#include <cstring>

#include "DataManager.hpp"
#include "utils.hpp"

namespace csd {

  auto DataManager::add_edge(const unsigned char* const rdfTerm, const std::size_t& from, const std::size_t& until, bool outNodeIsLeaf, const std::size_t& inNodeId, const std::size_t& outNodeId) -> std::size_t {
    possibly_realloc(&m_buffers.edges);
    const auto bufferIndex{m_buffers.edges.length++};
    m_buffers.edges.buf[bufferIndex] = {
      .outNodeIsLeaf = outNodeIsLeaf,
      .outNodeId = outNodeId,
      .inNodeId = inNodeId,
      .labelLength = until - from,
      .labelOffset = 0, // overwritten on save
      .deleted = false};
    possibly_realloc(&m_buffers.labels);
    m_buffers.labels.buf[bufferIndex] = static_cast<unsigned char*>(malloc(until - from));
    m_buffers.labels.length++;
    std::memcpy(m_buffers.labels.buf[bufferIndex], rdfTerm + from, until - from);
    m_stats.numEdges++;
    m_stats.numLabelBytes += (until - from);
    return m_mmapPointers.edges.length + bufferIndex;
  }

  auto DataManager::remove_edge(const std::size_t& edgeId) -> void {
    auto* e{get_edge(edgeId)};
    m_stats.numLabelBytes -= e->labelLength;
    m_stats.numEdges--;
    e->deleted = true;
  }
  auto DataManager::get_edge(const std::size_t& edgeId, bool dontThrowOnNotFound) const -> Edge* const {
    auto* const edge{get_item<struct Edge>(&m_mmapPointers.edges, &m_buffers.edges, edgeId)};
    if (edge->deleted) {
      if (!dontThrowOnNotFound) {
        throw std::runtime_error("Tried to get deleted edge ");
      }
    }
    return edge;
  }
  auto DataManager::edge_exists(const std::size_t& edgeId) const -> bool {
    auto* edge{get_item<struct Edge>(&m_mmapPointers.edges, &m_buffers.edges, edgeId)};
    return !edge->deleted;
  }
}
