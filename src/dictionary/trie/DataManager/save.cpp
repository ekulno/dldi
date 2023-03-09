#include <stdexcept>

#include <dictionary/trie/OutEdgeIterator.hpp>

#include "DataManager.hpp"
#include "utils.hpp"

auto numDeletedIdsBefore(std::size_t id, const std::vector<csd::Hole>& holes) -> std::size_t {
  if (holes.empty() || holes.at(0).start > id) {
    return 0;
  }
  if (holes.at(holes.size() - 1).start < id) {
    std::size_t result{holes.at(holes.size() - 1).cumulative};
    if (result == 0) {
      throw std::runtime_error("Cumulative of a hole is 0");
    }
    return result;
  }

  // look for two holes which the Id falls between.
  std::size_t searchMax{holes.size() - 1};
  std::size_t searchMin{0};
  std::size_t i{(searchMax - searchMin) / 2};

  while (true) {
    const auto hole{holes.at(i)};

    if (id < hole.start) {
      if (holes.at(i - 1).start < id) {
        // id is between the previous hole and this one
        return holes.at(i - 1).cumulative;
      }
      // id is smaller, cut search space in half. go to the left.
      searchMax = i - 1;
    } else {
      if (holes.at(i + 1).start > id) {
        // id is between this and the next hole
        return hole.cumulative;
      }
      searchMin = i + 1;
    }
    i = searchMax - (searchMax - searchMin) / 2;
  }
}
/**
 * @brief Compute new Id of an item, as it should be written to file.
 * This is necessary when we remove items, to ensure that references stay consistent.
 * @return std::size_t new Id
 */
auto get_new_id(std::size_t id, const std::vector<csd::Hole>& holes) -> std::size_t {
  auto numDeletionsBelowId{numDeletedIdsBefore(id, holes)};
  if (id < numDeletionsBelowId) {
    throw std::runtime_error("negative result Id");
  }
  return id - numDeletionsBelowId;
}

namespace csd {

  auto DataManager::computeLeafHoles() -> void {
    m_leafHolesComputed = true;

    std::size_t realId{0};
    std::size_t holeIndex{0};

    std::size_t numAppliedLeafNodeDeletions{0};
    for (std::size_t localId{0}; localId < m_mmapPointers.leaves.length && numAppliedLeafNodeDeletions < m_numNewLeafNodeDeletions; localId++) {
      const bool nodeIsDeleted{get_leafNode(localId, true)->occurences == 0};
      if (!nodeIsDeleted) {
        realId++;
        continue;
      }
      numAppliedLeafNodeDeletions++;
      while (holeIndex < m_loadTimeLeafHoles.size() && m_loadTimeLeafHoles.at(holeIndex).start + m_loadTimeLeafHoles.at(holeIndex).size < realId) {
        realId += m_loadTimeLeafHoles.at(holeIndex).size;
        m_finalLeafHoles.push_back(m_loadTimeLeafHoles.at(holeIndex));
        m_finalLeafHoles.at(m_finalLeafHoles.size() - 1).cumulative = (realId - localId);
        holeIndex++;
      }

      auto* hole{(m_loadTimeLeafHoles.empty() || m_loadTimeLeafHoles.size() <= holeIndex) ? nullptr : &m_loadTimeLeafHoles.at(holeIndex)};

      if (hole != nullptr && realId == hole->start - 1) {
        // expand left
        const Hole newHole{
          .start = hole->start - 1,
          .size = hole->size + 1,
          .cumulative = (m_finalLeafHoles.empty() ? 0 : m_finalLeafHoles.at(m_finalLeafHoles.size() - 1).cumulative) + hole->size + 1};
        m_finalLeafHoles.push_back(newHole);
        realId += newHole.size;
        continue;
      }

      if (!m_finalLeafHoles.empty() && m_finalLeafHoles.at(m_finalLeafHoles.size() - 1).start + m_finalLeafHoles.at(m_finalLeafHoles.size() - 1).size == realId) {
        m_finalLeafHoles.at(m_finalLeafHoles.size() - 1).size++;
        m_finalLeafHoles.at(m_finalLeafHoles.size() - 1).cumulative++;
        realId++;
        continue;
      }

      if (hole != nullptr && realId == hole->start + hole->size) {
        // expand right

        const Hole newHole{
          .start = hole->start,
          .size = hole->size + 1,
          .cumulative = (m_finalLeafHoles.empty() ? 0 : m_finalLeafHoles.at(m_finalLeafHoles.size() - 1).cumulative) + hole->size + 1};
        m_finalLeafHoles.push_back(newHole);
        realId++;
        continue;
      }

      if (!m_finalLeafHoles.empty() && realId == m_finalLeafHoles.at(m_finalLeafHoles.size() - 1).start + m_finalLeafHoles.at(m_finalLeafHoles.size() - 1).size) {
        m_finalLeafHoles.at(m_finalLeafHoles.size() - 1).size++;
        m_finalLeafHoles.at(m_finalLeafHoles.size() - 1).cumulative++;
        realId++;
        continue;
      }

      if (hole == nullptr || realId < hole->start) {
        // new hole
        const Hole newHole{
          .start = realId,
          .size = 1,
          .cumulative = (m_finalLeafHoles.empty() ? 0 : m_finalLeafHoles.at(m_finalLeafHoles.size() - 1).cumulative) + 1};
        m_finalLeafHoles.push_back(newHole);
        realId++;
      }
    }

    while (holeIndex < m_loadTimeLeafHoles.size()) {
      auto hole{m_loadTimeLeafHoles.at(holeIndex)};
      const auto cumulative{(m_finalLeafHoles.empty() ? 0 : m_finalLeafHoles.at(m_finalLeafHoles.size() - 1).cumulative) + hole.size};
      const Hole newHole{
        .start = hole.start,
        .size = hole.size,
        .cumulative = cumulative};
      m_finalLeafHoles.push_back(newHole);
      holeIndex++;
    }
  }
  auto DataManager::save(std::ostream& fp) -> void {
    if (!m_leafHolesComputed) {
      computeLeafHoles();
    }
    fp.write(reinterpret_cast<char*>(&(m_stats.numLeaves)), sizeof(m_stats.numLeaves));
    fp.write(reinterpret_cast<char*>(&(m_stats.numInternalNodes)), sizeof(m_stats.numInternalNodes));
    fp.write(reinterpret_cast<char*>(&(m_stats.numEdges)), sizeof(m_stats.numEdges));
    fp.write(reinterpret_cast<char*>(&(m_stats.numLabelBytes)), sizeof(m_stats.numLabelBytes));
    std::size_t numLeafHoles{m_finalLeafHoles.size()};
    fp.write(reinterpret_cast<char*>(&numLeafHoles), sizeof(numLeafHoles));

    auto internalNodeHoles{std::vector<csd::Hole>()};

    std::size_t numAppliedInternalNodeDeletions{0};
    for (std::size_t i = 0; i < m_mmapPointers.internals.length + m_buffers.internals.length && numAppliedInternalNodeDeletions < m_numInternalNodeDeletions; i++) {
      if (get_internalNode(i, true)->numOutEdges == 0) {
        add_hole_sequential(i, internalNodeHoles);
        numAppliedInternalNodeDeletions++;
      }
    }
    std::size_t numAppliedNewLeafNodeDeletions{0};
    std::vector<csd::Hole> mmapLeafHoles;
    for (std::size_t i = 0; i < m_mmapPointers.leaves.length && numAppliedNewLeafNodeDeletions < m_numNewLeafNodeDeletions; i++) {
      if (get_leafNode(i, true)->occurences == 0) {
        add_hole_sequential(i, mmapLeafHoles);
        numAppliedNewLeafNodeDeletions++;
      }
    }
    { // Write labels
      std::size_t num_labelbytes_written{0};
      for (std::size_t i{0}; i < m_mmapPointers.edges.length + m_buffers.edges.length; i++) {
        auto* edge{get_edge(i, true)};
        if (edge->deleted) {
          continue;
        }
        fp.write(reinterpret_cast<const char* const>(get_label(i, edge)), edge->labelLength);
        edge->labelOffset = num_labelbytes_written;
        num_labelbytes_written += edge->labelLength;
      }
      if (num_labelbytes_written != m_stats.numLabelBytes) {
        throw std::runtime_error("Wrote unexpected number of label bytes");
      }
    }

    { // Write edges
      std::size_t labelOffset{0};
      std::size_t num_written_edges{0};
      for (std::size_t i{0}; i < m_mmapPointers.edges.length + m_buffers.edges.length; i++) { // NOLINT(altera-unroll-loops)
        if (!edge_exists(i)) {
          continue;
        }
        auto* edge{get_edge(i)};
        edge->inNodeId = get_new_id(edge->inNodeId, internalNodeHoles);
        if (edge->outNodeIsLeaf) {
          edge->outNodeId = get_new_id(edge->outNodeId, mmapLeafHoles);
        } else {
          edge->outNodeId = get_new_id(edge->outNodeId, internalNodeHoles);
        }
        edge->labelOffset = labelOffset;
        labelOffset += edge->labelLength;
        fp.write(reinterpret_cast<const char* const>(edge), sizeof(*edge));

        edge->inNodeId = num_written_edges; // abuse this field to easily access the new Id later
        num_written_edges++;
      }
      if (num_written_edges != m_stats.numEdges) {
        std::cout << "Wrote " << num_written_edges << " but expected to write " << m_stats.numEdges << std::endl;
        throw std::runtime_error("Wrote unexpected number of edges");
      }
    }
    {
      std::size_t num_written_leafs{0};
      // Write leaf nodes
      for (std::size_t i{0}; i < m_mmapPointers.leaves.length + m_buffers.leaves.length; i++) { // NOLINT(altera-unroll-loops)
        auto* n{get_leafNode(i, true)};
        if (n->occurences > 0) {
          n->inEdge = get_edge(n->inEdge)->inNodeId; // hack, abused field
          fp.write(reinterpret_cast<const char* const>(n), sizeof(*n));
          num_written_leafs++;
        }
      }
      if (num_written_leafs != m_stats.numLeaves) {
        throw std::runtime_error("Wrote unexpected number of leaf nodes");
      }
    }

    {
      // Write leaf node holes
      // bool merge_border = false;
      std::size_t num_written_holes{0};
      for (auto& hole: m_finalLeafHoles) {
        auto* h{&hole};
        fp.write(reinterpret_cast<const char* const>(&(h->start)), sizeof(h->start));
        fp.write(reinterpret_cast<const char* const>(&(h->size)), sizeof(h->size));
        fp.write(reinterpret_cast<const char* const>(&(h->cumulative)), sizeof(h->cumulative));
        num_written_holes++;
      }
      if (num_written_holes != numLeafHoles) {
        throw std::runtime_error("Wrote unexpected number of leaf node holes");
      }
    }

    {
      // write out-edges
      // std::size_t num_written
      std::size_t num_written_edges{0};
      for (std::size_t i{0}; i < m_mmapPointers.internals.length + m_buffers.internals.length; i++) {
        auto* n{get_internalNode(i, true)};
        if (n->numOutEdges == 0) {
          continue;
        }
        auto it{OutEdgeIterator(i, this)};
        while (it.has_next()) {
          auto shifted{get_edge(it.read())->inNodeId}; // hack, abused field
          fp.write(reinterpret_cast<const char* const>(&shifted), sizeof(shifted));
          num_written_edges++;
          it.proceed();
        }
      }
      if (num_written_edges != m_stats.numEdges) {
        std::cout << "Wrote " << num_written_edges << " outedges but expected to write " << m_stats.numEdges << std::endl;
        throw std::runtime_error("Wrote unexpected number of edges");
      }
    }

    { // Write internal nodes
      std::size_t outEdgesOffset{0};
      std::size_t num_written_internalNodes{0};

      for (std::size_t i{0}; i < m_mmapPointers.internals.length + m_buffers.internals.length; i++) {
        auto* n{get_internalNode(i, true)};
        if (n->numOutEdges == 0) {
          // deleted
          continue;
        }
        if (i != 0) {
          n->inEdge = get_edge(n->inEdge)->inNodeId; // hack, abused field
        }

        n->outEdgesOffset = outEdgesOffset;
        fp.write(reinterpret_cast<const char* const>(n), sizeof(*n));
        outEdgesOffset += n->numOutEdges;
        num_written_internalNodes++;
      }
      if (num_written_internalNodes != m_stats.numInternalNodes) {
        throw std::runtime_error("Wrote unexpected number of internal nodes");
      }
    }
  }
}
