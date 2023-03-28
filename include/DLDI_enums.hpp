#ifndef DLDI_ENUMS_HPP
#define DLDI_ENUMS_HPP


#include <stdexcept>
#include <string>

namespace dldi {

  /**
   * Subject, predicate, object
  */
  using TriplePattern = std::tuple<std::size_t, std::size_t, std::size_t>;
  
  enum class TripleOrder {
    SPO,
    SOP,
    PSO,
    POS,
    OSP
  };

  enum class SourceType {
    PlainTextLinkedData_Sorted,
    PlainTextLinkedData_Unsorted,
    DynamicLinkedDataIndex
  };

  enum class TripleTermPosition {
    subject,
    predicate,
    object
  };
  class EnumMapping {
  public:
    static constexpr TripleOrder TRIPLE_ORDERS[]{TripleOrder::SPO, TripleOrder::SOP, TripleOrder::PSO, TripleOrder::POS, TripleOrder::OSP};

    static auto order_to_string(const dldi::TripleOrder& order) -> std::string {
      if (order == TripleOrder::SPO)
        return "SPO";
      if (order == TripleOrder::SOP)
        return "SOP";
      if (order == TripleOrder::PSO)
        return "PSO";
      if (order == TripleOrder::POS)
        return "POS";
      if (order == TripleOrder::OSP)
        return "OSP";
      throw std::runtime_error("Unrecognized order");
    }

    static auto position_to_string(const dldi::TripleTermPosition& position) -> std::string {
      if (position == dldi::TripleTermPosition::subject) {
        return "subject";
      }
      if (position == dldi::TripleTermPosition::predicate) {
        return "predicate";
      }
      if (position == dldi::TripleTermPosition::object) {
        return "object";
      }
      throw std::runtime_error("Unrecognized triple-term position");
    }
  };
}
#endif 