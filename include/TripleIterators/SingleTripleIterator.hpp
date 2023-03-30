#ifndef DLDI_SINGLE_TRIPLE_ITERATOR_HPP
#define DLDI_SINGLE_TRIPLE_ITERATOR_HPP

#include <Iterator.hpp>
#include <QuantifiedTriple.hpp>

namespace dldi {

  class SingleTripleIterator : public Iterator<QuantifiedTriple> {
  public:
    SingleTripleIterator(const dldi::TripleOrder& order,
                         const std::tuple<std::size_t, std::size_t, std::size_t>& filter,
                         const std::shared_ptr<const dldi::DictionariesHandle> dicts) {
      if (order != dldi::TripleOrder::SPO) {
        throw std::runtime_error("SingleTripleIterator only supports SPO");
      }
      m_has_next = true;
      m_next = dldi::QuantifiedTriple{std::get<0>(filter), std::get<1>(filter), std::get<2>(filter), 1};
    }

    auto inner_proceed() -> void override {
      m_has_next = false;
    }
  };
}
#endif