#include <cstddef>
#include <iostream>
#include <stdexcept>

#include "LabelComparator.hpp"

namespace csd {

  LabelComparator::LabelComparator(const std::string& search_term)
    : m_search_term{search_term} {
  }
  auto LabelComparator::compare(const unsigned char* const edgeLabel, const std::size_t& edgeLabelLength, const std::size_t& searchTermOffset) -> TermRelation {
    const auto* const s2{m_search_term.c_str() + searchTermOffset};
    const auto s2len{m_search_term.size() + 1 - searchTermOffset};
    for (m_mismatch_index = 0; m_mismatch_index < edgeLabelLength && m_mismatch_index < s2len; m_mismatch_index++) {
      if (edgeLabel[m_mismatch_index] != s2[m_mismatch_index]) {
        break;
      }
    }
    if (m_mismatch_index == 0) {
      m_labelIsLexicographicallyAfterKey = edgeLabel[0] > s2[0];
      return TermsShareNoPrefix;
    } else if (m_mismatch_index == edgeLabelLength) {
      // no difference was found within the range of s1.
      // s1 is either identical to s2,
      // or s2 is longer than s1 and s1 is a prefix of s2.

      return (m_mismatch_index == s2len) ? TermsAreEqual : FirstTermIsPrefixOfSecondTerm;
    } else if (m_mismatch_index == s2len) {
      // no difference was found within the range of s2.
      // or s1 is longer than s2 and s2 is a prefix of s1.
      return SecondTermIsPrefixOfFirstTerm;
    } else {
      // the two terms share a common prefix,
      // but neither is the prefix of the other
      return TermsSharePrefix;
    }
  }

  auto LabelComparator::mismatchIndex() const -> std::size_t {
    return m_mismatch_index;
  }
  auto LabelComparator::labelIsLexicographicallyAfterKey() const -> bool {
    return m_labelIsLexicographicallyAfterKey;
  }
}
