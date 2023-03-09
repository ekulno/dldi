#ifndef PATH_CHAR_ITERATOR_HPP
#define PATH_CHAR_ITERATOR_HPP

#include <cstddef>

#include "./DataManager/DataManager.hpp"

namespace csd {

  class PathCharIterator {
  public:
    PathCharIterator(const DataManager* const data, const TriePath& triePath);
    auto next() -> unsigned char;
    [[nodiscard]] auto has_next() const -> bool;

  private:
    const TriePath m_triePath;
    const DataManager* const m_data;
    std::size_t m_index;
    std::pair<std::size_t, const Edge*> m_segment;
    std::size_t m_nextSegmentIndex;
    unsigned char* m_label;
    auto setNextLabel() -> void;
  };
}
#endif
