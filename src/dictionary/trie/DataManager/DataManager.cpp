#include <dictionary/trie/Trie.hpp>

#include <dictionary/trie/DataTypes.hpp>

#include "DataManager.hpp"

#define INITIAL_CAPCITY 1

namespace csd {

  DataManager::DataManager()
    : m_stats{.numLabelBytes = 0, .rawDataBytes = 0, .numLeaves = 0, .numInternalNodes = 0, .numEdges = 0},
      m_buffers{
        .leaves{
          .buf{static_cast<struct LeafNode* const>(malloc(sizeof(struct LeafNode) * INITIAL_CAPCITY))},
          .capacity{INITIAL_CAPCITY},
          .length{0}},
        .internals{
          .buf{static_cast<struct InternalNode* const>(malloc(sizeof(struct InternalNode) * INITIAL_CAPCITY))},
          .capacity{INITIAL_CAPCITY},
          .length{0}},
        .edges{
          .buf{static_cast<struct Edge* const>(malloc(sizeof(struct Edge) * INITIAL_CAPCITY))},
          .capacity{INITIAL_CAPCITY},
          .length{0}},
        .labels{
          .buf{static_cast<unsigned char** const>(malloc(sizeof(unsigned char*) * INITIAL_CAPCITY))},
          .capacity{INITIAL_CAPCITY},
          .length{0}}},
      m_mmapPointers{
        .leaves{
          .ptr{nullptr},
          .length{0}},
        .internals{
          .ptr{nullptr},
          .length{0}},
        .edges{
          .ptr{nullptr},
          .length{0}},
        .labels{
          .ptr{nullptr},
          .length{0}},
        .outEdgeIds{
          .ptr{nullptr},
          .length{0}}},
      m_outEdgesMap{std::make_unique<std::unordered_map<std::size_t, NewOutEdgesList>>()},
      m_loadTimeLeafHoles{std::vector<csd::Hole>()},
      m_finalLeafHoles{std::vector<csd::Hole>()},
      m_numNewLeafNodeDeletions{0},
      m_numInternalNodeDeletions{0},
      m_leafHolesComputed{false} {
  }
  DataManager::~DataManager() {
    for (std::size_t i = 0; i < m_buffers.edges.length; i++) {
      free(m_buffers.labels.buf[i]);
    }
    free(m_buffers.edges.buf);
    free(m_buffers.internals.buf);
    free(m_buffers.leaves.buf);
    free(m_buffers.labels.buf);
  }
  auto DataManager::getStats() const -> const TrieStats* const {
    return &m_stats;
  }
  auto DataManager::getMmapPointers() const -> const MmapPointers* const {
    return &m_mmapPointers;
  }
}
