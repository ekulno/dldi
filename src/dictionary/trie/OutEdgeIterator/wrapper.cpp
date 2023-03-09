#include <algorithm>
#include <cstddef>
#include <stdexcept>

#include <dictionary/trie/OutEdgeIterator.hpp>

#include "../DataManager/DataManager.hpp"

namespace csd {
  OutEdgeIterator::OutEdgeIterator(const std::size_t& nodeId, const DataManager* const data)
    : m_it_mmapped{OutEdgeIterator_mmapped(nodeId, data)},
      m_it_memory{OutEdgeIterator_memory(nodeId, data)},
      m_data{data},
      m_next_is_mmapped{true} {
    sort_iterators();
    m_has_next = (m_next_is_mmapped) ? m_it_mmapped.has_next() : m_it_memory.has_next();
    if (m_has_next) {
      m_next = (m_next_is_mmapped) ? m_it_mmapped.read() : m_it_memory.read();
    }
  }

  auto OutEdgeIterator::inner_proceed() -> void {
    (m_next_is_mmapped) ? m_it_mmapped.proceed() : m_it_memory.proceed();
    sort_iterators();
    m_has_next = (m_next_is_mmapped) ? m_it_mmapped.has_next() : m_it_memory.has_next();
    if (m_has_next) {
      m_next = (m_next_is_mmapped) ? m_it_mmapped.read() : m_it_memory.read();
    }
  }
  auto OutEdgeIterator::sort_iterators() -> void {
    if (!m_it_mmapped.has_next()) {
      m_next_is_mmapped = false;
      return;
    }
    if (!m_it_memory.has_next()) {
      m_next_is_mmapped = true;
      return;
    }

    const auto ch1{m_data->get_label(m_it_mmapped.read())[0]};
    const auto ch2{m_data->get_label(m_it_memory.read())[0]};

    m_next_is_mmapped = (ch1 < ch2);
  }
}
