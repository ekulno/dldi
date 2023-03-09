#include <iostream>
#include <stdexcept>

#include <dictionary/trie/Trie.hpp>

#include "DataManager/DataManager.hpp"
#include "PathCharIterator.hpp"

namespace csd {

  PathCharIterator::PathCharIterator(const DataManager* const data, const TriePath& triePath)
    : m_triePath{triePath},
      m_data{data},
      m_index{0},
      m_nextSegmentIndex{0},
      m_label{nullptr} // initialized by setNextLabel()
  {
    setNextLabel();
  }
  auto PathCharIterator::setNextLabel() -> void {
    m_segment = m_triePath.at(m_triePath.size() - m_nextSegmentIndex - 1);
    m_label = m_data->get_label(m_segment.first, m_segment.second);
    m_nextSegmentIndex++;
    m_index = 0;
  }
  auto PathCharIterator::next() -> unsigned char {
    const auto result{m_label[m_index++]};
    if (m_index == m_segment.second->labelLength && m_nextSegmentIndex < m_triePath.size()) {
      setNextLabel();
    }
    return result;
  }
  auto PathCharIterator::has_next() const -> bool {
    return (m_index < m_segment.second->labelLength || m_nextSegmentIndex < m_triePath.size());
  }
}
