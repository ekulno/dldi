#include <execution>
#include <fstream>
#include <iostream>

#include "./TriplesWriter.hpp"

namespace dldi {
  class TriplesComparator_lex {
  public:
    TriplesComparator_lex(const Dictionary& subjects, const Dictionary& predicates, const Dictionary& objects, const dldi::TripleOrder& order)
      : m_subjects{subjects}, m_predicates{predicates}, m_objects{objects}, m_order{order} {
    }

    [[nodiscard]] auto operator()(const QuantifiedTriple& lhs, const QuantifiedTriple& rhs) -> bool {
      if (lhs.subject() == 0 || rhs.subject() == 0) {
        throw std::runtime_error("Invalid triple (sub)");
      }
      if (lhs.predicate() == 0 || rhs.predicate() == 0) {
        throw std::runtime_error("Invalid triple (pred)");
      }
      if (lhs.object() == 0 || rhs.object() == 0) {
        throw std::runtime_error("Invalid triple (obj)");
      }

      return lhs.smaller_or_equal_to(rhs, m_order, m_subjects, m_predicates, m_objects);
    }

  private:
    const Dictionary& m_subjects;
    const Dictionary& m_predicates;
    const Dictionary& m_objects;
    const dldi::TripleOrder& m_order;
  };

  auto TriplesWriter::add(const std::size_t& subject, const std::size_t& predicate, const std::size_t& object) -> void {
    if (subject == 0 || predicate == 0 || object == 0) {
      throw std::runtime_error("Tried to add an invalid triple");
    }
    m_triples.push_back(QuantifiedTriple{subject, predicate, object, 1});
  }

  auto TriplesWriter::sort(const dldi::Dictionary& subjects, const dldi::Dictionary& predicates, const dldi::Dictionary& objects, const dldi::TripleOrder& order) -> void {
    const auto comparator{TriplesComparator_lex(subjects, predicates, objects, order)};
    std::sort(m_triples.begin(), m_triples.end(), comparator);
  }
  auto TriplesWriter::save(const std::filesystem::path& path) -> void {
    std::ofstream out{path, std::ios::binary | std::ios::trunc};
    if (!out.good()) {
      throw std::runtime_error("Error opening file to save dictionary");
    }

    for (const auto& triple: m_triples) {
      if (triple.quantity() == 0) {
        continue;
      }
      if (triple.subject() == 0 || triple.predicate() == 0 || triple.object() == 0) {
        throw std::runtime_error("Tried to save invalid triple. Implementation error.");
      }
      out.write(reinterpret_cast<const char*>(&triple), sizeof(triple));
    }
    out.close();
  }
}
