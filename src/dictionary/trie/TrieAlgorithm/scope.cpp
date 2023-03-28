

#include "../LabelComparator.hpp"
#include "../TrieNavigator.hpp"
#include "TrieAlgorithm.hpp"

namespace csd {

  auto TrieAlgorithm::get_scope(const DataManager* const data, const std::string& prefix) -> std::tuple<std::size_t, bool, bool> {
    if (prefix.size() == 0) {
      // if prefix is empty, then we should match everything,
      // starting at the root.
      return {0, true, false};
    }

    auto navigator = TrieNavigator(data);
    LabelComparator comparator{prefix};
    std::size_t keyOffset{0};

    while (true) {
      const auto comparisonResult{comparator.compare(navigator.label(), navigator.edge()->labelLength, keyOffset)};
      if (comparisonResult == TermsShareNoPrefix) {
        // no prefix match here, but maybe on a following edge.
        if (comparator.labelIsLexicographicallyAfterKey() || !navigator.mayGoRight()) {
          // there is no following edge, so no results. 
          return {0, false, false};
        }
        navigator.goRight();
        continue;
      }

      if (comparisonResult == FirstTermIsPrefixOfSecondTerm) {
        // the path's label is a prefix of the search term
        // continue from this child.
        keyOffset += comparator.mismatchIndex();
        if (keyOffset==prefix.size()){
          const auto outNodeId{navigator.edge()->outNodeId};
          return {outNodeId, true, navigator.edge()->outNodeIsLeaf};
        }
        navigator.goDown();
        continue;
      }

      if (comparisonResult == TermsAreEqual || comparisonResult == SecondTermIsPrefixOfFirstTerm) {
        const auto outNodeId{navigator.edge()->outNodeId};
        // there is exactly one result. 
        return {outNodeId, true, navigator.edge()->outNodeIsLeaf};
      }

      // comparisonResult == TermsSharePrefix
      /**
       * Example: 
       * 
       * prefix: charming
       * 
       * 0 char x
       *        x mander $
       *        x izard  $
       * 
       * char(x)mander$ shares a prefix with charming: `charm`. 
       * 
       * If there would be any term matching the `charming` prefix, 
       * Then we would expect another split, as such: 
       * 
       * 
       * 0 char x
       *        x m y
       *            y ander $
       *            y ing [...]
       *        x izard  $
       * 
       * But, this was apparently not the case. 
       * This means there is no match. 
       * 
      */
      return {0, false, false};
    }
  }
}
