#include <algorithm>
#include <execution>
#include <fstream>
#include <iostream>

#include "./TriplesWriter.hpp"

namespace dldi {
  class TriplesComparator_lex {
  public:
    TriplesComparator_lex(const dldi::DictionariesHandle& dicts, const dldi::TripleOrder& order, bool ignore_primary)
      : m_dicts{dicts}, m_order{order}, m_ignore_primary{ignore_primary} {
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

      return lhs.smaller_or_equal_to(rhs, m_order, m_dicts, m_ignore_primary);
    }

  private:
    const dldi::DictionariesHandle& m_dicts;
    const dldi::TripleOrder& m_order;
    const bool m_ignore_primary;
  };

  auto TriplesWriter::add(const std::size_t& subject, const std::size_t& predicate, const std::size_t& object) -> void {
    if (subject == 0 || predicate == 0 || object == 0) {
      throw std::runtime_error("Tried to add an invalid triple");
    }
    m_triples.push_back(QuantifiedTriple{subject, predicate, object, 1});
  }

  auto TriplesWriter::sort(const dldi::DictionariesHandle& dicts, const dldi::TripleOrder& order, bool ignore_primary) -> void {
    const TriplesComparator_lex comparator{dicts, order, ignore_primary};
    std::sort(m_triples.begin(), m_triples.end(), comparator);
  }

  auto inline get_outstream(const std::filesystem::path& output_path, const std::string& key) -> std::ofstream {
    const std::filesystem::path path{output_path.string() + "/" + key + ".triples-component"};
    std::ofstream out{path, std::ios::binary | std::ios::trunc};

    if (!out.good()) {
      throw std::runtime_error("Error opening file to save triples component: " + path.string());
    }
    return out;
  }

  inline auto write_sizet(std::ofstream& out, const std::size_t& val) -> void {
    out.write(reinterpret_cast<const char*>(&val), sizeof(val));
  }

  auto TriplesWriter::save(
    const std::filesystem::path& output_path,
    const dldi::DictionariesHandle& dicts)
    -> void {
    for (auto order: {
           dldi::TripleOrder::SPO,
           dldi::TripleOrder::SOP,
           dldi::TripleOrder::PSO,
           dldi::TripleOrder::POS,
           dldi::TripleOrder::OSP}) {

      const bool same_as_previous_primary{order == dldi::TripleOrder::SOP || order == dldi::TripleOrder::POS};

      // std::cout << "sort \n";
      sort(dicts, order, same_as_previous_primary);
      // std::cout << "sorted! \n";

      const auto [primary_position, secondary_position, tertiary_position]{EnumMapping::order_to_positions(order)};

      // We can reuse the primary ids list for two pairs of orders: SPO,SOP and PSO,POS.
      const auto should_write_primary_ids{!same_as_previous_primary};
      std::ofstream primary_ids_out;
      if (should_write_primary_ids) {
        primary_ids_out = get_outstream(output_path, EnumMapping::position_to_string(primary_position) + ".primary-ids");
      }

      auto secondary_ids_out{get_outstream(output_path, EnumMapping::order_to_string(order) + ".secondary-ids")};
      auto secondary_ref_out{get_outstream(output_path, EnumMapping::order_to_string(order) + ".secondary-refs")};
      std::size_t secondary_ref_length{0};
      auto tertiary_ids_out{get_outstream(output_path, EnumMapping::order_to_string(order) + ".tertiary-ids")};
      auto tertiary_ref_out{get_outstream(output_path, EnumMapping::order_to_string(order) + ".tertiary-refs")};
      std::size_t tertiary_ref_length{0};

      std::size_t i{0};
      while (i < m_triples.size()) {
        if (m_triples.at(i).quantity() == 0) {
          ++i;
          continue;
        }

        const auto primary_term{m_triples.at(i).term(primary_position)};

        if (primary_term == 0) {
          throw std::runtime_error("Tried to save invalid triple. Implementation error.");
        }

        // write the ID of the primary-positioned term (default:subject) which we're currently concerned with.
        if (should_write_primary_ids) {
          write_sizet(primary_ids_out, primary_term);
        }
        // write index of the beginning of the secondary-level ID ranges belonging to this primary-level term.
        write_sizet(secondary_ref_out, secondary_ref_length);

        // for all triples with the given primary-level term:
        // std::size_t i{begin_index};
        while (i < m_triples.size() && m_triples.at(i).term(primary_position) == primary_term) {
          if (m_triples.at(i).quantity() == 0) {
            ++i;
            continue;
          }

          const auto secondary_term{m_triples.at(i).term(secondary_position)};

          if (secondary_term == 0) {
            throw std::runtime_error("Tried to save invalid triple. Implementation error.");
          }

          write_sizet(secondary_ids_out, secondary_term);
          write_sizet(tertiary_ref_out, tertiary_ref_length);

          for (; i < m_triples.size() && m_triples.at(i).term(secondary_position) == secondary_term && m_triples.at(i).term(primary_position) == primary_term; ++i) {
            if (m_triples.at(i).quantity() == 0) {
              ++i;
              continue;
            }
            const auto tertiary_term{m_triples.at(i).term(tertiary_position)};
            if (tertiary_term == 0) {
              throw std::runtime_error("Tried to save invalid triple. Implementation error.");
            }
            write_sizet(tertiary_ids_out, tertiary_term);
            ++tertiary_ref_length;
          }

          ++secondary_ref_length;
        }
      }
      if (should_write_primary_ids) {
        primary_ids_out.close();
      }
      secondary_ids_out.close();
      secondary_ref_out.close();
      tertiary_ids_out.close();
      tertiary_ref_out.close();
    }
  }
}
