#include <cstddef>
#include <tuple>
#include <vector>

#include <dictionary/trie/OutEdgeIterator.hpp>
#include <dictionary/trie/TermIterator.hpp>

#include "DataManager/DataManager.hpp"
#include "TrieAlgorithm/TrieAlgorithm.hpp"

namespace csd {

  TermIterator::TermIterator(const DataManager* const data, const std::string& prefix)
    : m_iterators{std::vector<csd::OutEdgeIterator>()}, m_data{data} {
    const auto scope_info{TrieAlgorithm::get_scope(data, prefix)};

    const auto at_least_one_result{std::get<1>(scope_info)};
    if (!at_least_one_result) {
      m_has_next = false;
      return;
    }

    m_scope = std::get<0>(scope_info);

    const auto is_singleton{std::get<2>(scope_info)};

    if (is_singleton) {
      m_has_next = true;
      m_next = m_scope;
    } else {
      m_iterators.emplace_back(OutEdgeIterator{m_scope, data});
      inner_proceed();
    }
  }

  auto TermIterator::inner_proceed() -> void {
    while (!m_iterators.empty()) {
      while (m_iterators.at(m_iterators.size() - 1).has_next()) {
        const auto edgeId{m_iterators.at(m_iterators.size() - 1).read()};
        const auto* const edge{m_data->get_edge(edgeId)};
        m_iterators.at(m_iterators.size() - 1).proceed();
        if (edge->outNodeIsLeaf) {
          m_next = edge->outNodeId;
          m_has_next = true;
          return;
        }
        m_iterators.emplace_back(OutEdgeIterator(edge->outNodeId, m_data));
      }
      m_iterators.pop_back();
    }
    m_has_next = false;
  }
}
