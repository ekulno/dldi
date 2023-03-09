#include <cstddef>
#include <stdexcept>
#include <vector>

#include <dictionary/trie/DataTypes.hpp>
#include <dictionary/trie/OutEdgeIterator.hpp>

#include "./TrieNavigator.hpp"
#include "DataManager/DataManager.hpp"

namespace csd {
  TrieNavigator::TrieNavigator(const DataManager* const data, const std::size_t& startingNodeId)
    : m_data{data},
      m_upper{nullptr},
      m_lower{nullptr},
      m_edge{nullptr},
      m_it{new OutEdgeIterator(startingNodeId, data)} {
    if (!mayGoRight()) {
      std::cout << startingNodeId << std::endl;
      throw std::runtime_error("Tried to go right but isn't allowed");
    }
    goRight();
  }

  TrieNavigator::~TrieNavigator() {
    delete m_it;
  }

  auto TrieNavigator::mayGoRight() -> bool {
    return m_it->has_next();
  }
  auto TrieNavigator::mayGoDown() const -> bool {
    return !m_edge->outNodeIsLeaf;
  }
  auto TrieNavigator::goRight() -> void {
    if (!mayGoRight()) {
      throw std::runtime_error("(TrieNavigator::goRight) Tried to go right when its not allowed");
    }
    if (m_it == nullptr || !m_it->has_next()) {
      throw std::runtime_error("problem???");
    }
    m_edgeId = m_it->read();
    m_it->proceed();
    m_edge = m_data->get_edge(m_edgeId);
  }
  auto TrieNavigator::goDown() -> void {
    if (!mayGoDown()) {
      throw std::runtime_error("Tried to go down but isn't allowed");
    }
    m_upper = m_lower;
    m_lower = nullptr;
    delete m_it;
    m_it = new OutEdgeIterator(m_edge->outNodeId, m_data);
    if (!mayGoRight()) {
      throw std::runtime_error("Not allowed to go right in goDown");
    }
    goRight();
  }
  auto TrieNavigator::edge() const -> Edge* const {
    return m_edge;
  }
  auto TrieNavigator::label() const -> unsigned char* const {
    return m_data->get_label(m_edgeId, m_edge);
  }
  auto TrieNavigator::outNode() -> InternalNode* const {
    if (m_lower == nullptr) {
      m_lower = m_data->get_internalNode(m_edge->outNodeId);
    }
    return m_lower;
  }
  auto TrieNavigator::inNode() -> InternalNode* const {
    if (m_upper == nullptr) {
      m_upper = m_data->get_internalNode(m_edge->inNodeId);
    }
    return m_upper;
  }
  auto TrieNavigator::leaf(bool throwOnDeleted) const -> LeafNode* const {
    return m_data->get_leafNode(m_edge->outNodeId, !throwOnDeleted);
  }
  auto TrieNavigator::edgeId() const -> std::size_t {
    return m_edgeId;
  }

  auto TrieNavigator::refreshEdge() -> void {
    m_edge = m_data->get_edge(edgeId());
  }
}
