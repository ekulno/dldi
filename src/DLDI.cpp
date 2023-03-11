#include <iostream>
#include <filesystem>

#include <DLDI.hpp>

#include "./triples/TriplesReader.hpp"
#include <dictionary/Dictionary.hpp>

namespace dldi {
  DLDI::DLDI(const std::filesystem::path& data_dir)
    : m_datadir{data_dir} {
      if (!std::filesystem::is_directory(data_dir)){
        throw std::runtime_error("Not a directory: " + data_dir.string());
      }
      dldi::Dictionary::validate_dir(data_dir);
      dldi::TriplesReader::validate_dir(data_dir);
  }

  auto DLDI::get_triples(const dldi::TripleOrder& order) const -> std::shared_ptr<TriplesReader> {
    if (order == dldi::TripleOrder::SPO)
      return m_triples_spo;
    if (order == dldi::TripleOrder::SOP)
      return m_triples_sop;
    if (order == dldi::TripleOrder::PSO)
      return m_triples_pso;
    if (order == dldi::TripleOrder::POS)
      return m_triples_pos;
    if (order == dldi::TripleOrder::OSP)
      return m_triples_osp;
    if (order == dldi::TripleOrder::OPS)
      return m_triples_ops;
    throw std::runtime_error("Unaccounted-for order");
  }

  auto DLDI::query_ptr(const dldi::TriplePattern& pattern) const -> std::shared_ptr<dldi::TriplesIterator> {
    const auto order{decide_order_from_triple_pattern(pattern)};
    const auto triples{get_triples(order)};
    if (triples == nullptr) {
      throw std::runtime_error("Triples not loaded!");
    }
    return triples->query_ptr(pattern, *m_subjects, *m_predicates, *m_objects);
  }
  auto DLDI::query_ptr(const dldi::TripleOrder& order) const -> std::shared_ptr<dldi::TriplesIterator> {
    const auto triples{get_triples(order)};
    if (triples == nullptr) {
      throw std::runtime_error("Triples not loaded!");
    }
    return triples->query_ptr(dldi::TriplePattern{0, 0, 0}, *m_subjects, *m_predicates, *m_objects);
  }

  auto DLDI::query(const std::string prefix, const dldi::TripleTermPosition position) const -> csd::TermStringIterator {
    const auto dict{
      position == dldi::TripleTermPosition::subject ? m_subjects : position == dldi::TripleTermPosition::predicate ? m_predicates :
                                                                                                                     m_objects};
    if (dict == nullptr) {
      throw std::runtime_error("The dictionary isn't loaded, can't perform prefix query.");
    }
    return dict->query(prefix);
  }

  auto DLDI::query(const std::string prefix, bool subjects, bool predicates, bool objects) const -> dldi::AnyPositionTermIterator {
    std::vector<csd::TermStringIterator> iterators;
    if (subjects) {
      const auto it{query(prefix, dldi::TripleTermPosition::subject)};
      if (it.has_next()) {
        iterators.push_back(it);
      }
    }
    if (predicates) {
      const auto it{query(prefix, dldi::TripleTermPosition::predicate)};
      if (it.has_next())
        iterators.push_back(it);
    }
    if (objects) {
      const auto it{query(prefix, dldi::TripleTermPosition::object)};
      if (it.has_next())
        iterators.push_back(it);
    }
    return dldi::AnyPositionTermIterator(iterators);
  }
  auto DLDI::string_to_id(const std::string& term, const dldi::TripleTermPosition& position) const -> std::size_t {
    return get_dict(position)->string_to_id(term);
  }
  auto DLDI::id_to_string(const std::size_t& id, const dldi::TripleTermPosition& position) const -> std::string {
    return get_dict(position)->id_to_string(id);
  }
  auto DLDI::ensure_loaded(const dldi::TripleTermPosition& position) -> void {
    if (get_dict(position)) {
      return;
    }

    const auto dict{std::make_shared<Dictionary>(dldi::Dictionary::dictionary_file_path(m_datadir, position))};
    
    if (position == dldi::TripleTermPosition::subject) {
      m_subjects = dict;
    } else if (position == dldi::TripleTermPosition::predicate) {
      m_predicates = dict;
    } else if (position == dldi::TripleTermPosition::object) {
      m_objects = dict;
    } else {
      throw std::runtime_error("Unrecognized position");
    }
  }

  auto DLDI::prepare_for_query(const dldi::TriplePattern& pattern) -> void {
    const auto order{decide_order_from_triple_pattern(pattern)};
    ensure_loaded_triples(order);
  }

  auto DLDI::ensure_loaded_triples(const dldi::TripleOrder& order) -> void {
    if (get_triples(order)) {
      return;
    }
    const auto reader{std::make_shared<TriplesReader>(dldi::TriplesReader::triples_file_path(m_datadir, order))};
    if (order == dldi::TripleOrder::SPO) {
      m_triples_spo = reader;
    } else if (order == dldi::TripleOrder::SOP) {
      m_triples_sop = reader;
    } else if (order == dldi::TripleOrder::PSO) {
      m_triples_pso = reader;
    } else if (order == dldi::TripleOrder::POS) {
      m_triples_pos = reader;
    } else if (order == dldi::TripleOrder::OSP) {
      m_triples_osp = reader;
    } else if (order == dldi::TripleOrder::OPS) {
      m_triples_ops = reader;
    } else {
      throw std::runtime_error("Unexpected order");
    }
  }
}
