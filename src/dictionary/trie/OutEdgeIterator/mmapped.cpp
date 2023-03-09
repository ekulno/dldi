#include <cstddef>
#include <stdexcept>

#include <dictionary/trie/OutEdgeIterator.hpp>

#include "../DataManager/DataManager.hpp"

namespace csd {
  OutEdgeIterator_mmapped::OutEdgeIterator_mmapped(const std::size_t& nodeId, const DataManager* const data)
    : m_data{data},
      m_ptr{nullptr},
      m_tooFar{nullptr} {
    const auto* const mmapPointers{m_data->getMmapPointers()};

    if (nodeId >= mmapPointers->internals.length) {
      // no out-edges for this node within mmapped area.
      m_has_next = false;
      return;
    }

    m_ptr = mmapPointers->outEdgeIds.ptr + m_data->get_internalNode(nodeId)->outEdgesOffset;
    if (nodeId == mmapPointers->internals.length - 1) {
      // if this is the last out-edge, then we will have gone too far if we go past the last out-edge id.
      m_tooFar = mmapPointers->outEdgeIds.ptr + mmapPointers->outEdgeIds.length;
    } else {
      m_tooFar = mmapPointers->outEdgeIds.ptr + m_data->get_internalNode(nodeId + 1, 1)->outEdgesOffset;
    }
    m_next = *m_ptr;
    m_has_next = true;
  }

  auto OutEdgeIterator_mmapped::inner_proceed() -> void {
    if (m_ptr == nullptr || m_ptr >= m_tooFar) {
      m_has_next = false;
      m_ptr = nullptr;
      return;
    }

    ++m_ptr;

    while (m_ptr < m_tooFar) {
      if (m_data->edge_exists(*m_ptr)) {
        m_has_next = true;
        m_next = *m_ptr;
        return;
      } else {
        ++m_ptr;
      }
    }
    m_ptr = nullptr;
    m_has_next = false;
  }
}
