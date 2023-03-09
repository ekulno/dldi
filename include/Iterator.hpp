#ifndef ITERATOR_HPP
#define ITERATOR_HPP

#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace dldi {
  template <class T>
  class Iterator {
  public:
    auto read() const -> T {
      if (!has_next()) {
        throw std::runtime_error("There is no next.");
      }
      return m_next;
    }
    auto has_next() const -> bool {
      return m_has_next;
    }

    auto proceed() -> void {
      if (!has_next()) {
        throw std::runtime_error("Can't proceed when there is no next");
      }
      inner_proceed();
    }

  protected:
    virtual auto inner_proceed() -> void = 0;
    T m_next{};
    bool m_has_next{false};
  };
}
#endif