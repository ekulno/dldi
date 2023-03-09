#ifndef TERM_ITERATOR_HPP
#define TERM_ITERATOR_HPP

#include <cstddef>
#include <string>
#include <vector>

#include <Iterator.hpp>
#include <dictionary/trie/OutEdgeIterator.hpp>

namespace csd {
  class DataManager;
  class TermIterator : public dldi::Iterator<std::size_t> {
  public:
    TermIterator(const DataManager* const data, const std::string& prefix);
    auto inner_proceed() -> void override;

  private:
    std::size_t m_scope;
    std::vector<csd::OutEdgeIterator> m_iterators;
    const DataManager* m_data;
  };
}
#endif
