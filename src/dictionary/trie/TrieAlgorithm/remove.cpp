#include <cstddef>
#include <cstring>
#include <stdexcept>

#include <dictionary/trie/DataTypes.hpp>
#include <dictionary/trie/OutEdgeIterator.hpp>

#include "../LabelComparator.hpp"
#include "../TrieNavigator.hpp"
#include "./TrieAlgorithm.hpp"

namespace csd {

  auto disconnect_leaf(DataManager* const data, InternalNode* const node, std::size_t currentNodeId, std::size_t outedge_Id) -> void {
    data->remove_edge(outedge_Id);
    data->remove_outedge(currentNodeId, outedge_Id);
    node->numOutEdges--;

    // we now consider merging away the internal node.

    if (currentNodeId == 0) {
      // // root node. merging is not applicable.
      return;
    }
    if (node->numOutEdges == 0) {
      throw std::runtime_error("Should have already merged away this node");
    }
    if (node->numOutEdges > 1) {
      // we can't merge away the internal node since there's still > 1 outedge.
      return;
    }

    // (node->numOutEdges == 1)
    // merge away this node. node->parent gets node->child as direct descendant
    auto it{OutEdgeIterator(currentNodeId, data)};
    if (!it.has_next()) {
      std::cout << currentNodeId << std::endl;
      throw std::runtime_error("Expected there to be exactly 1 out-edge");
    }

    const auto oldOutEdgeId{it.read()};
    it.proceed();
    if (it.has_next()) {
      std::cout << currentNodeId << std::endl;
      throw std::runtime_error("Expected there to be exactly 1 out-edge");
    }

    const auto oldInEdgeId{node->inEdge};
    const auto* const oldInEdge{data->get_edge(oldInEdgeId)};
    const auto* oldOutEdge{data->get_edge(oldOutEdgeId)};
    const auto* const oldInLabel{data->get_label(oldInEdgeId, oldInEdge)};
    const auto* const oldOutLabel{data->get_label(oldOutEdgeId, oldOutEdge)};

    const auto newLength{oldInEdge->labelLength + oldOutEdge->labelLength};

    auto* newLabel{static_cast<unsigned char*>(malloc(newLength))};
    memcpy(newLabel, oldInLabel, oldInEdge->labelLength);
    memcpy(newLabel + oldInEdge->labelLength, oldOutLabel, oldOutEdge->labelLength);
    const auto inNodeId{oldInEdge->inNodeId};
    const auto eid{data->add_edge(newLabel, 0, newLength, oldOutEdge->outNodeIsLeaf, inNodeId, oldOutEdge->outNodeId)};
    free(newLabel);
    oldOutEdge = data->get_edge(oldOutEdgeId); // got to re-get it because pointers might have changed after adding an edge
    if (oldOutEdge->outNodeIsLeaf) {
      data->get_leafNode(oldOutEdge->outNodeId)->inEdge = eid;
    } else {
      data->get_internalNode(oldOutEdge->outNodeId)->inEdge = eid;
    }

    data->add_outEdge(inNodeId, eid);
    data->remove_edge(oldInEdgeId);
    data->remove_edge(oldOutEdgeId);
    data->get_internalNode(inNodeId)->numOutEdges--;
    data->remove_outedge(inNodeId, oldInEdgeId);
    data->remove_outedge(currentNodeId, oldOutEdgeId);

    data->remove_internalNode(currentNodeId);
  }

  auto TrieAlgorithm::remove(DataManager* data, const std::size_t& id, const std::size_t& occurences) -> bool {
    auto* leaf{data->get_leafNode(id, true)};
    if (leaf->occurences == 0) {
      throw std::runtime_error("Already deleted");
    }
    if (leaf->occurences > occurences) {
      // we remove some occurences, but more remain so no change to the tree structure.
      leaf->occurences -= occurences;
      return false;
    }

    // occurrences==0. remove the leaf node.
    const auto* const edge{data->get_edge(leaf->inEdge)};
    auto* const parentNode{data->get_internalNode(edge->inNodeId)};
    data->remove_leafNode(id);
    disconnect_leaf(data, parentNode, edge->inNodeId, leaf->inEdge);
    return true;
  }
}
