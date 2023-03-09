#include <cstddef>
#include <string.h>
#include <tuple>
#include <vector>

#include <DLDI.hpp>

namespace dldi {
  AnyPositionTermIterator::AnyPositionTermIterator(std::vector<csd::TermStringIterator>& iterators)
    : m_iterators{iterators} {
    if (m_iterators.size() > 3) {
      throw std::runtime_error("Expected at most three iterators");
    }
    sort_iterators();
  }
  auto AnyPositionTermIterator::next() -> std::string {
    if (!has_next()) {
      throw std::runtime_error("No next, call has_next() first.");
    }
    const auto result{m_iterators.at(0).read()};
    do {
      m_iterators.at(0).proceed();
      if (m_iterators.at(0).has_next() && m_iterators.at(0).read() == result) {
        throw std::runtime_error("duplicate terms, unexpected");
      }
      if (!m_iterators.at(0).has_next()) {
        // the iterator we just took an element from is now empty.
        // remove it from the vector.
        m_iterators.erase(m_iterators.begin());
      }
      sort_iterators();
    } while (!m_iterators.empty() && m_iterators.at(0).read().first == result.first);
    return result.first;
  }
  auto AnyPositionTermIterator::peek() const -> std::string {
    if (!has_next()) {
      throw std::runtime_error("No next, call has_next() first.");
    }
    return m_iterators.at(0).read().first;
  }
  auto AnyPositionTermIterator::has_next() const -> bool {
    return !m_iterators.empty();
  }
  auto AnyPositionTermIterator::sort_iterators() -> void {
    // now, ensure that our iterators are still ordered
    // by the lex comparison of each iterator's next term.

    if (m_iterators.size() > 1) {
      const auto peek_a{m_iterators.at(0).read()};
      const auto peek_b{m_iterators.at(1).read()};
      {
        const auto comparison{strcmp(peek_a.first.c_str(), peek_b.first.c_str())};
        if (comparison > 0) {
          // peek_a is lexicographically after peek_b, so they should swap places.
          iter_swap(m_iterators.begin(), m_iterators.begin() + 1);
        }
      }
      // peek_a is now the second element.
      if (m_iterators.size() == 3) {
        // check whether we need to move peek_a even further back, to the end.
        const auto peek_c{m_iterators.at(2).read()};
        const auto comparison{strcmp(peek_a.first.c_str(), peek_c.first.c_str())};
        if (comparison > 0) {
          // peek_a is lexicographically after peek_c, so they should swap places.
          iter_swap(m_iterators.begin() + 1, m_iterators.begin() + 2);
        }
      }
    }
  }
}
