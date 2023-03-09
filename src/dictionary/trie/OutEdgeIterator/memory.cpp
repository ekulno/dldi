#include <cstddef>
#include <stdexcept>

#include <dictionary/trie/OutEdgeIterator.hpp>

#include "../DataManager/DataManager.hpp"

namespace csd {

  OutEdgeIterator_memory::OutEdgeIterator_memory(const std::size_t& nodeId, const DataManager* const data)
    : m_newEdges{nullptr}, m_newEdgesIndex{0}, m_data{data} {
    m_newEdgesIndex = 0;
    m_newEdges = m_data->getNewOutEdges(nodeId);
    m_has_next = m_newEdges != nullptr && !m_newEdges->empty();
    if (m_has_next) {
      m_next = m_newEdges->at(m_newEdgesIndex);
    }
  }

  auto OutEdgeIterator_memory::inner_proceed() -> void {
    ++m_newEdgesIndex;
    if (m_newEdges == nullptr || m_newEdges->empty() || m_newEdgesIndex >= m_newEdges->size()) {
      m_has_next = false;
      return;
    }
    if (!m_data->edge_exists(m_newEdges->at(m_newEdgesIndex))) {
      throw std::runtime_error("newEdges contains a deleted edge.");
    }
    m_has_next = true;
    m_next = m_newEdges->at(m_newEdgesIndex);
  }
}
