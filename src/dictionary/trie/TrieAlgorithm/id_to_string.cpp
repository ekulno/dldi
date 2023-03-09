#include <cstring>
#include <vector>

#include "./TrieAlgorithm.hpp"

namespace csd {
  auto TrieAlgorithm::compile_path_label(const DataManager* const data, const TriePath& triePath, bool dontThrowOnNotFound) -> std::string {
    // todo evaluate whether it pays off to pre-compute the string length.

    std::size_t lengths_sum{0};
    for (auto edge: triePath) {
      lengths_sum += edge.second->labelLength;
    }
    std::string result;
    result.reserve(lengths_sum);
    for (int index = triePath.size() - 1; index >= 0; index--) {
      const auto edge{triePath.at(index)};
      const auto* const edgeLabel{data->get_label(edge.first, edge.second, dontThrowOnNotFound)};
      result += std::string{reinterpret_cast<const char* const>(edgeLabel), edge.second->labelLength};
    }
    return result;
  }

  auto TrieAlgorithm::id_to_string(const DataManager* const data, const std::size_t& id, bool dontThrowOnNotFound) -> std::string {
    return compile_path_label(data, extract_path(data, id), dontThrowOnNotFound);
  }

  std::size_t maxDepth{5};

  auto TrieAlgorithm::extract_path(const DataManager* const data, const std::size_t& leafNodeId, bool dontThrowOnNotFound) -> TriePath {
    auto triePath{TriePath()};
    // give an initial guesstimate for the capacity, to avoid needless reallocations
    triePath.reserve(maxDepth);
    const auto* const leaf{data->get_leafNode(leafNodeId, dontThrowOnNotFound)};
    auto* edge{data->get_edge(leaf->inEdge, dontThrowOnNotFound)};
    {
      const auto inEdgeId{leaf->inEdge};
      triePath.emplace_back(std::pair{inEdgeId, edge});
    }
    while (edge->inNodeId > 0) { // NOLINT(altera-unroll-loops)
      const auto* const internalNode{data->get_internalNode(edge->inNodeId, dontThrowOnNotFound)};
      edge = data->get_edge(internalNode->inEdge, dontThrowOnNotFound);
      const auto inEdgeId{internalNode->inEdge};
      triePath.emplace_back(std::pair{inEdgeId, edge});
    }
    if (triePath.size() > maxDepth) {
      maxDepth = triePath.size();
    }
    return triePath;
  }
}
