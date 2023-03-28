#include <cstring>

#include "DataManager.hpp"
#include "utils.hpp"

namespace csd {

  auto DataManager::add_edge(const unsigned char* const rdfTerm, const std::size_t& from, const std::size_t& until, bool outNodeIsLeaf, const std::size_t& inNodeId, const std::size_t& outNodeId) -> std::size_t {
    if (outNodeIsLeaf){
      if (rdfTerm[until-1]!='\0'){
        throw std::runtime_error("Expected last char of rdfTerm to be null char, since outNodeIsLeaf.");
      }
    }
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
  auto DataManager::edge_to_string(const std::size_t& edge_id) const -> std::string {
    // this function is only used for debugging. 
    const auto*const edge{get_edge(edge_id)};
    const auto*const label{get_label(edge_id)};
    std::string s{};
    for (std::size_t i{0}; i<edge->labelLength; i ++ ){
      if (label[i]=='\0') {
        if (i!=edge->labelLength-1){
          throw std::runtime_error("Found null-byte in the middle of a edge label");
        }
        break;
      }
      s.push_back(label[i]);
    }
    return s;
  }
}
