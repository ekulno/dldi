#ifndef OUTEDGE_IT_HPP
#define OUTEDGE_IT_HPP

#include <cstddef>
#include <vector>
#include <memory>

#include <Iterator.hpp>


namespace csd {

  using NewOutEdgesList = std::shared_ptr<std::vector<std::size_t>>;

  class DataManager;

  /**
   * @brief Iterator over outedges in mmapped area
   *
   */
  class OutEdgeIterator_mmapped : public dldi::Iterator<std::size_t> {
  public:
    OutEdgeIterator_mmapped(const std::size_t& nodeId, const DataManager* const data);
    auto inner_proceed() -> void override;

  private:
    const DataManager* const m_data;
    std::size_t* m_ptr;
    std::size_t* m_tooFar;
  };

  /**
   * @brief Iterator over outedges added during the current run
   *
   */
  class OutEdgeIterator_memory : public dldi::Iterator<std::size_t> {
  public:
    OutEdgeIterator_memory(const std::size_t& nodeId, const DataManager* const data);
    auto inner_proceed() -> void override;

  private:
    const DataManager* const m_data;
    NewOutEdgesList m_newEdges;
    std::size_t m_newEdgesIndex;
  };

  /**
   * @brief Iterator over all outedges
   *
   */
  class OutEdgeIterator : public dldi::Iterator<std::size_t> {
  public:
    OutEdgeIterator(const std::size_t& nodeId, const DataManager* const data);
    auto inner_proceed() -> void override;

  private:
    const DataManager* const m_data;
    std::vector<dldi::Iterator<std::size_t>*> m_iterators;

    auto sort_iterators() -> void;

    bool m_next_is_mmapped;
    OutEdgeIterator_mmapped m_it_mmapped;
    OutEdgeIterator_memory m_it_memory;
  };
}
#endif
