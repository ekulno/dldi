#ifndef TERM_STRING_ITERATOR_HPP
#define TERM_STRING_ITERATOR_HPP

#include <cstddef>
#include <string>
#include <vector>

#include <Iterator.hpp>
#include <dictionary/trie/TermIterator.hpp>

namespace csd {
  class DataManager;
  class TermStringIterator : public dldi::Iterator<std::pair<std::string, std::size_t>> {
  public:
    TermStringIterator(const DataManager* const data, const std::string& prefix);

    // protected:
    auto inner_proceed() -> void override;

  private:
    std::size_t m_scope;
    TermIterator m_termiterator;
    const DataManager* m_data;
  };
}
#endif
