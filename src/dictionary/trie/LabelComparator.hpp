#ifndef LABEL_COMPARATOR_HPP
#define LABEL_COMPARATOR_HPP

#include <cstddef>
#include <string>

namespace csd {

  enum TermRelation {
    TermsShareNoPrefix,
    TermsAreEqual,
    FirstTermIsPrefixOfSecondTerm,
    SecondTermIsPrefixOfFirstTerm,
    TermsSharePrefix
  };

  class LabelComparator {
  public:
    LabelComparator(const std::string& search_term);
    ~LabelComparator() = default;
    auto compare(const unsigned char* const edgeLabel, const std::size_t& edgeLabelLength, const std::size_t& searchTermOffset) -> TermRelation;
    [[nodiscard]] auto mismatchIndex() const -> std::size_t;
    [[nodiscard]] auto labelIsLexicographicallyAfterKey() const -> bool;

  private:
    const std::string m_search_term;
    std::size_t m_mismatch_index;
    bool m_labelIsLexicographicallyAfterKey;
  };
}
#endif
