#include <filesystem>
#include <iostream>

#include <DLDI.hpp>
#include <dictionary/Dictionary.hpp>

#include "./triples/TriplesReader.hpp"

namespace dldi {
  DLDI::DLDI(const std::filesystem::path& data_dir)
    : m_datadir{data_dir},
    // not sure why this statement is needed, thought this would be created automatically upon construction...
    m_dicts{std::make_shared<DictionariesHandle>()}{
    if (!std::filesystem::is_directory(data_dir)) {
      throw std::runtime_error("Not a directory: " + data_dir.string());
    }
    dldi::Dictionary::validate_dir(data_dir);
    dldi::TriplesReader::validate_dir(data_dir);
  }

  auto DLDI::search(const dldi::TriplePattern& pattern) const -> std::shared_ptr<dldi::TriplesIterator> {
    return m_triples_reader->search(pattern, m_dicts);
  }

  auto DLDI::terms(const std::string prefix, const dldi::TripleTermPosition position) const -> std::shared_ptr<csd::TermStringIterator> {
    const auto dict{get_dict(position)};
    if (dict == nullptr) {
      throw std::runtime_error("The dictionary isn't loaded, can't perform prefix query.");
    }
    return dict->query(prefix);
  }

  auto DLDI::terms(const std::string prefix, bool subjects, bool predicates, bool objects) const -> std::shared_ptr<dldi::AnyPositionTermIterator> {
    std::vector<std::shared_ptr<csd::TermStringIterator>> iterators;
    if (subjects) {
      const auto it{terms(prefix, dldi::TripleTermPosition::subject)};
      if (it->has_next()) {
        iterators.push_back(it);
      }
    }
    if (predicates) {
      const auto it{terms(prefix, dldi::TripleTermPosition::predicate)};
      if (it->has_next())
        iterators.push_back(it);
    }
    if (objects) {
      const auto it{terms(prefix, dldi::TripleTermPosition::object)};
      if (it->has_next())
        iterators.push_back(it);
    }
    return std::make_shared<dldi::AnyPositionTermIterator>(iterators);
  }
  auto DLDI::string_to_id(const std::string& term, const dldi::TripleTermPosition& position) const -> std::size_t {
    const auto dict{get_dict(position)};
    if (!dict) {
      throw std::runtime_error("Dict isn't loaded");
    }
    return dict->string_to_id(term);
  }
  auto DLDI::id_to_string(const std::size_t& id, const dldi::TripleTermPosition& position) const -> std::string {
    const auto dict{get_dict(position)};
    if (!dict) {
      throw std::runtime_error("Dict isn't loaded");
    }
    return dict->id_to_string(id);
  }
  auto DLDI::ensure_loaded(const dldi::TripleTermPosition& position) -> void {
    m_dicts->ensure_loaded(position, m_datadir);
  }

  auto DLDI::prepare_for_query(const dldi::TriplePattern& pattern) -> void {
    const auto order{QuantifiedTriple::decide_order_from_triple_pattern(pattern)};
    ensure_loaded_triples(order);
  }

  auto DLDI::ensure_loaded_triples(const dldi::TripleOrder& order) -> void {
    if (get_triples()) {
      return;
    }
    m_triples_reader = std::make_shared<TriplesReader>(m_datadir);
  }
}
