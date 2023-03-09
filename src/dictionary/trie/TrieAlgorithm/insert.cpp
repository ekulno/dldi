#include <cstddef>
#include <stdexcept>

#include "../DataManager/DataManager.hpp"
#include "../LabelComparator.hpp"
#include "../TrieNavigator.hpp"
#include "./TrieAlgorithm.hpp"

namespace csd {

  auto addFirstKey(DataManager* data, const std::string& rdfTerm, const std::size_t& occurrences) -> std::size_t {
    std::size_t rootId{0};
    if (data->getStats()->numInternalNodes == 0) {
      // it's not the case that we've previously added some data, and then removed it.
      // if we do that, we'll still have 1 internal node (root) and no leaves.
      rootId = data->add_internalNode(0);
    }
    const auto edgeId{data->add_edge(reinterpret_cast<const unsigned char* const>(rdfTerm.c_str()), 0, rdfTerm.size(), true, rootId, 0)};
    const auto leafNodeId{data->add_leafNode(edgeId, occurrences)};
    data->get_edge(edgeId)->outNodeId = leafNodeId;
    data->add_outEdge(rootId, edgeId);
    return leafNodeId;
  }

  auto insertLeafNode(DataManager* data, const unsigned char* const key, std::size_t keyOffset, std::size_t keyLength, std::size_t inNodeId, const std::size_t& occurrences) -> std::size_t {
    const auto newEdgeId{data->add_edge(key, keyOffset, keyLength, true, inNodeId, 0)};
    auto* newEdge{data->get_edge(newEdgeId)};
    const std::size_t newLeafNodeId{data->add_leafNode(newEdgeId, occurrences)};
    newEdge->outNodeId = newLeafNodeId;
    data->add_outEdge(inNodeId, newEdgeId);
    return newLeafNodeId;
  }

  auto TrieAlgorithm::insert(DataManager* data, const std::string& term, const std::size_t& occurrences) -> std::pair<std::size_t, bool> {
    // check for special case: first string
    if (data->getStats()->numLeaves == 0) {
      const std::pair<std::size_t, bool> result{addFirstKey(data, term, occurrences), true};
      return result;
    }

    auto navigator = TrieNavigator(data);

    const auto* key{reinterpret_cast<const unsigned char* const>(term.c_str())};
    const auto keyLength{term.size()};

    LabelComparator comparator{term};
    std::size_t keyOffset{0};

    while (true) {
      const auto comparisonResult{comparator.compare(navigator.label(), navigator.edge()->labelLength, keyOffset)};
      if (comparisonResult == TermsShareNoPrefix) {
        // no prefix match here, but maybe on a following edge.
        if (comparator.labelIsLexicographicallyAfterKey() || !navigator.mayGoRight()) {
          const std::pair<std::size_t, bool> result{insertLeafNode(data, key, keyOffset, keyLength, navigator.edge()->inNodeId, occurrences), true};
          return result;
        }
        navigator.goRight();
        continue;
      }

      if (comparisonResult == TermsAreEqual) {
        // Match, already inserted. Increment occurences and return the outnode
        navigator.leaf()->occurences += occurrences;
        const auto resultId{navigator.edge()->outNodeId};
        const std::pair<std::size_t, bool> result{resultId, false};
        return result;
      }

      if (comparisonResult == FirstTermIsPrefixOfSecondTerm) {
        // e->outNodeId is an internal node, and e->label is a prefix of `rdfTerm`.
        // Insert the remaining chars as a new child under this->outNode.
        keyOffset += navigator.edge()->labelLength;
        navigator.goDown();
        continue;
      }
      if (comparisonResult == SecondTermIsPrefixOfFirstTerm) {
        // All keywords should be null-terminated.
        // If the input label is a prefix of this label,
        // then that must mean this label has a null-byte inside it.
        // that is not allowed.
        throw std::runtime_error("Should not reach this");
      }

      // COMMON_PREFIX : a strict prefix of e->label is a strict prefix of rdfTerm.
      // We must break up this edge, as such:
      //
      //        a --e1--> b
      //             =>
      // a --e1--> x , x --e2--> b
      //               x --e3-> c
      //

      const auto xId{data->add_internalNode(navigator.edgeId())};

      const auto e2Id{data->add_edge(
        navigator.label(),
        comparator.mismatchIndex(),
        navigator.edge()->labelLength,
        navigator.edge()->outNodeIsLeaf,
        xId,
        navigator.edge()->outNodeId)};

      // need to re-get it, since we might have reallocated
      navigator.refreshEdge();

      data->add_outEdge(xId, e2Id);

      if (navigator.edge()->outNodeIsLeaf) {
        navigator.leaf()->inEdge = e2Id;
      } else {
        navigator.outNode()->inEdge = e2Id;
      }

      data->shrink_label(navigator.edgeId(), comparator.mismatchIndex());
      navigator.edge()->outNodeId = xId;
      navigator.edge()->outNodeIsLeaf = false;

      keyOffset += comparator.mismatchIndex();

      const std::pair<std::size_t, bool> result{insertLeafNode(data, key, keyOffset, keyLength, xId, occurrences), true};
      return result;
    }
  }
}
