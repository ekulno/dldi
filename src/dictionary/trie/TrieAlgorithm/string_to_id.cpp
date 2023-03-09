#include <stdexcept>

#include "../LabelComparator.hpp"
#include "../TrieNavigator.hpp"
#include "TrieAlgorithm.hpp"

namespace csd {

  auto TrieAlgorithm::string_to_id(const DataManager* const data, const std::string& term) -> std::pair<std::size_t, std::size_t> {
    auto navigator = TrieNavigator(data);
    LabelComparator comparator{term};
    std::size_t keyOffset{0};

    while (true) {
      const auto comparisonResult{comparator.compare(navigator.label(), navigator.edge()->labelLength, keyOffset)};

      if (comparisonResult == TermsShareNoPrefix) {
        if (comparator.labelIsLexicographicallyAfterKey() || !navigator.mayGoRight()) {
          throw StringNotFoundException();
        }
        navigator.goRight();
        continue;
      }
      if (comparisonResult == FirstTermIsPrefixOfSecondTerm) {
        keyOffset += comparator.mismatchIndex();
        navigator.goDown();
        continue;
      }
      if (comparisonResult == TermsAreEqual) {
        const auto outNodeId{navigator.edge()->outNodeId};
        return std::pair<std::size_t, std::size_t>{data->internalToExposedId(outNodeId), outNodeId};
      }
      throw StringNotFoundException();
    }
  }
}
