#include <stdexcept>

#include "DataManager.hpp"

namespace csd {

  auto DataManager::add_outEdge(const std::size_t& nodeId, const std::size_t& edgeId) const -> void {
    if (!m_outEdgesMap->contains(nodeId)) {
      std::pair<std::size_t, std::vector<std::size_t>*> entry(nodeId, new std::vector<std::size_t>());
      m_outEdgesMap->insert(entry);
    }
    get_internalNode(nodeId, true)->numOutEdges++;

    auto* outEdges{m_outEdgesMap->at(nodeId)};
    if (outEdges->empty()) {
      outEdges->push_back(edgeId);
      return;
    }
    // maintain lexicographic ordering
    unsigned char c1{get_label(edgeId)[0]};
    for (std::size_t i{0}; i < outEdges->size(); i++) {
      unsigned char c2{get_label(outEdges->at(i))[0]};
      if (c1 < c2) {
        outEdges->insert(outEdges->begin() + static_cast<long>(i), edgeId);
        return;
      }
    }
    outEdges->push_back(edgeId);
  }

  auto DataManager::remove_outedge(const std::size_t& nodeId, const std::size_t& outEdgeId) const -> void {
    if (!m_outEdgesMap->contains(nodeId)) {
      return;
    }
    std::vector<std::size_t>* v{m_outEdgesMap->at(nodeId)};
    if (v->empty()) {
      delete v;
      m_outEdgesMap->erase(nodeId);
      return;
    }
    for (std::size_t index{0}; index < v->size(); index++) {
      if (v->at(index) == outEdgeId) {
        v->erase(v->begin() + index);
        break;
      }
    }
  }

  auto DataManager::getNewOutEdges(const std::size_t& nodeId) const -> std::vector<std::size_t>* const {
    if (m_outEdgesMap == nullptr || !m_outEdgesMap->contains(nodeId)) {
      return nullptr;
    }
    return m_outEdgesMap->at(nodeId);
  }
}
