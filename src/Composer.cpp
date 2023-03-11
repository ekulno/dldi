#include <algorithm>
#include <memory>

#include "./BinaryStreamWriter.hpp"
#include "./Composer.hpp"
#include "./triples/TriplesReader.hpp"
#include "./triples/TriplesWriter.hpp"
#include <dictionary/Dictionary.hpp>

namespace {
  auto merge_dictionaries(
    std::vector<std::shared_ptr<dldi::DLDI>>& dldis,
    const dldi::TripleTermPosition& position,
    const std::size_t& dldi_index) -> void {
    dldis.at(dldi_index)->ensure_loaded(position);
    auto aggregate_dict{dldis.at(dldi_index)->get_dict(position)};
    for (std::size_t i{0}; i < dldis.size(); i++) {
      if (i == dldi_index)
        continue;

      auto dldi{dldis.at(i)};
      dldi->ensure_loaded(position);
      auto terms{dldi->query("", position)};
      while (terms.has_next()) {
        const auto term{terms.read()};
        aggregate_dict->add(term.first, term.second);
        terms.proceed();
      }
    }
  }

  auto apply_dict_removals(
    std::shared_ptr<dldi::DLDI> main_dldi,
    std::vector<std::shared_ptr<dldi::DLDI>>& rem_dldis,
    const dldi::TripleTermPosition& position) -> void {
    for (const auto& rem_dldi: rem_dldis) {
      auto terms{rem_dldi->query("", position)};
      while (terms.has_next()) {
        const auto term{terms.read()};
        main_dldi->get_dict(position)->remove(term.first, term.second);
        terms.proceed();
      }
    }
  }

  class DT {
  public:
    DT(

      std::shared_ptr<dldi::TriplesIterator> iterator,
      std::shared_ptr<dldi::Dictionary> subjects,
      std::shared_ptr<dldi::Dictionary> predicates,
      std::shared_ptr<dldi::Dictionary> objects)
      : m_iterator{iterator},
        m_subjects{subjects},
        m_predicates{predicates},
        m_objects{objects} {
    }
    auto get_dict(const dldi::TripleTermPosition& position) const -> std::shared_ptr<dldi::Dictionary> {
      if (position == dldi::TripleTermPosition::subject)
        return m_subjects;
      if (position == dldi::TripleTermPosition::predicate)
        return m_predicates;
      if (position == dldi::TripleTermPosition::object)
        return m_objects;
      throw std::runtime_error("Unrecognized position");
    }
    auto compare(const std::shared_ptr<DT> other, const dldi::TripleTermPosition& position) const -> int {
      return get_dict(position)->compare(m_iterator->read().term(position), other->m_iterator->read().term(position), other->get_dict(position));
    }
    auto has_next() const -> bool {
      return m_iterator->has_next();
    }
    auto read() const -> dldi::QuantifiedTriple {
      return m_iterator->read();
    }
    auto proceed() -> void {
      return m_iterator->proceed();
    }

  private:
    std::shared_ptr<dldi::TriplesIterator> m_iterator;
    std::shared_ptr<dldi::Dictionary> m_subjects;
    std::shared_ptr<dldi::Dictionary> m_predicates;
    std::shared_ptr<dldi::Dictionary> m_objects;
  };

  inline auto triple_order_to_position_list(const dldi::TripleOrder& order) -> std::vector<dldi::TripleTermPosition> {
    if (order == dldi::TripleOrder::SPO)
      return {dldi::TripleTermPosition::subject, dldi::TripleTermPosition::predicate, dldi::TripleTermPosition::object};
    if (order == dldi::TripleOrder::SOP)
      return {dldi::TripleTermPosition::subject, dldi::TripleTermPosition::object, dldi::TripleTermPosition::predicate};
    if (order == dldi::TripleOrder::POS)
      return {dldi::TripleTermPosition::predicate, dldi::TripleTermPosition::object, dldi::TripleTermPosition::subject};
    if (order == dldi::TripleOrder::PSO)
      return {dldi::TripleTermPosition::predicate, dldi::TripleTermPosition::subject, dldi::TripleTermPosition::object};
    if (order == dldi::TripleOrder::OSP)
      return {dldi::TripleTermPosition::object, dldi::TripleTermPosition::subject, dldi::TripleTermPosition::predicate};
    if (order == dldi::TripleOrder::OPS)
      return {dldi::TripleTermPosition::object, dldi::TripleTermPosition::predicate, dldi::TripleTermPosition::subject};
    throw std::runtime_error("Unrecognized order");
  }

  class IdTriplesIteratorComparator {
  public:
    IdTriplesIteratorComparator(const dldi::TripleOrder& order)
      : m_order{order}, m_positions{triple_order_to_position_list(m_order)} {
    }

    [[nodiscard]] auto operator()(const std::shared_ptr<DT> lhs, const std::shared_ptr<DT> rhs) -> bool {
      // true: lhs should preceede rhs
      // false: rhs should preceede lhs

      if (lhs->has_next() && !rhs->has_next())
        return true;
      if (!lhs->has_next() && rhs->has_next())
        return false;
      if (!lhs->has_next() && !rhs->has_next())
        return true;
      // neither is depleted, check which's iterator is lexicographically first. 

      for (const auto position: m_positions) {
        const auto comparison{lhs->compare(rhs, position)};
        if (comparison != 0)
          return comparison < 0;
      }
      return true;
      throw std::runtime_error("Unrecognized order");
    }

  private:
    const dldi::TripleOrder m_order;
    const std::vector<dldi::TripleTermPosition> m_positions;
  };

  inline auto largest_dict_index(std::vector<std::shared_ptr<dldi::DLDI>>& dldis, const dldi::TripleTermPosition& position) -> std::size_t {
    std::size_t largest{0};
    std::size_t index_of_largest{0};
    for (std::size_t i{0}; i < dldis.size(); i++) {
      auto dldi{dldis.at(i)};
      dldi->ensure_loaded(position);
      auto dict{dldi->get_dict(position)};
      const auto size{dict->size()};
      if (size > largest) {
        largest = size;
        index_of_largest = i;
      }
    }
    return index_of_largest;
  }

  class RemappedAggregateTriplesIterator : public dldi::Iterator<dldi::QuantifiedTriple> {
  public:
    RemappedAggregateTriplesIterator(
      std::vector<std::shared_ptr<dldi::DLDI>>& dldis,
      const std::shared_ptr<dldi::Dictionary> mapto_subjects_dict,
      const std::shared_ptr<dldi::Dictionary> mapto_predicates_dict,
      const std::shared_ptr<dldi::Dictionary> mapto_objects_dict,
      const dldi::TripleOrder& order)
      : m_dldis{dldis},
        m_mapto_subjects_dict{mapto_subjects_dict},
        m_mapto_predicates_dict{mapto_predicates_dict},
        m_mapto_objects_dict{mapto_objects_dict},
        m_order{order},
        m_comparator{IdTriplesIteratorComparator{order}} {
      for (auto dldi: dldis) {
        dldi->ensure_loaded_triples(order);
        dldi->ensure_loaded(dldi::TripleTermPosition::subject);
        dldi->ensure_loaded(dldi::TripleTermPosition::predicate);
        dldi->ensure_loaded(dldi::TripleTermPosition::object);
        auto query_iterator{dldi->query_ptr(order)};

        m_dts.push_back(
          std::make_shared<DT>(
            query_iterator,
            dldi->get_dict(dldi::TripleTermPosition::subject),
            dldi->get_dict(dldi::TripleTermPosition::predicate),
            dldi->get_dict(dldi::TripleTermPosition::object)));
      }
      std::sort(m_dts.begin(), m_dts.end(), m_comparator);
      m_has_next = !dldis.empty();
      if (has_next()) {
        proceed();
      }
    }

  protected:
    auto inner_proceed() -> void override {
      if (m_dts.empty()) {
        m_has_next = false;
        return;
      }
      m_has_next = true;
      auto dt{m_dts.at(0)};
      auto id_next{dt->read()};

      const dldi::QuantifiedTriple id_final{
        dt->get_dict(dldi::TripleTermPosition::subject) == m_mapto_subjects_dict ? id_next.subject() : (m_mapto_subjects_dict->string_to_id(dt->get_dict(dldi::TripleTermPosition::subject)->id_to_string(id_next.subject()))),
        dt->get_dict(dldi::TripleTermPosition::predicate) == m_mapto_predicates_dict ? id_next.predicate() : (m_mapto_predicates_dict->string_to_id(dt->get_dict(dldi::TripleTermPosition::predicate)->id_to_string(id_next.predicate()))),
        dt->get_dict(dldi::TripleTermPosition::object) == m_mapto_objects_dict ? id_next.object() : (m_mapto_objects_dict->string_to_id(dt->get_dict(dldi::TripleTermPosition::object)->id_to_string(id_next.object()))),
        id_next.quantity()};

      m_next = id_final;

      dt->proceed();
      if (!dt->has_next()) {
        m_dts.erase(m_dts.begin());
      } else {
        std::sort(m_dts.begin(), m_dts.end(), m_comparator);
      }
    }

  private:
    std::vector<std::shared_ptr<dldi::DLDI>>& m_dldis;
    const std::shared_ptr<dldi::Dictionary> m_mapto_subjects_dict;
    const std::shared_ptr<dldi::Dictionary> m_mapto_predicates_dict;
    const std::shared_ptr<dldi::Dictionary> m_mapto_objects_dict;
    std::vector<std::shared_ptr<DT>> m_dts;
    IdTriplesIteratorComparator m_comparator;
    const dldi::TripleOrder& m_order;
  };

  auto merge_triples(
    std::vector<std::shared_ptr<dldi::DLDI>>& additions,
    std::vector<std::shared_ptr<dldi::DLDI>>& removals,
    const std::filesystem::path& output_path,
    const std::size_t& largest_subject_index,
    const std::size_t& largest_predicate_index,
    const std::size_t& largest_object_index,
    const dldi::TripleOrder& order) -> void {
    RemappedAggregateTriplesIterator add_iterator{
      additions,
      additions.at(largest_subject_index)->get_dict(dldi::TripleTermPosition::subject),
      additions.at(largest_predicate_index)->get_dict(dldi::TripleTermPosition::predicate),
      additions.at(largest_object_index)->get_dict(dldi::TripleTermPosition::object),
      order};
    RemappedAggregateTriplesIterator rem_iterator{
      removals,
      additions.at(largest_subject_index)->get_dict(dldi::TripleTermPosition::subject),
      additions.at(largest_predicate_index)->get_dict(dldi::TripleTermPosition::predicate),
      additions.at(largest_object_index)->get_dict(dldi::TripleTermPosition::object),
      order};
    dldi::StreamWriter<dldi::QuantifiedTriple> triples{dldi::TriplesReader::triples_file_path(output_path, order)};
    while (add_iterator.has_next()) {
      auto add_next{add_iterator.read()};

      add_iterator.proceed();
      while (add_iterator.has_next() && add_next.equals(add_iterator.read())) {
        add_next.set_quantity(add_next.quantity() + add_iterator.read().quantity());
        add_iterator.proceed();
      }
      while (rem_iterator.has_next() && add_next.equals(rem_iterator.read())) {
        if (add_next.quantity() < rem_iterator.read().quantity()) {
          throw std::runtime_error("Removing more than we're adding.");
        }
        add_next.set_quantity(add_next.quantity() - rem_iterator.read().quantity());
        rem_iterator.proceed();
      }
      if (add_next.quantity() > 0) {
        triples.write(add_next);
      }
    }
    if (rem_iterator.has_next()) {
      std::cout << "unexpectedly undepleted: " << std::endl;
      rem_iterator.read().print();
      std::cout << std::endl;
      throw std::runtime_error("Rem iterator not depleted");
    }
  }
}
namespace dldi {
  auto Composer::zip(dldi::SourceInfoVector& additions, dldi::SourceInfoVector& removals, const std::filesystem::path& output_dir) -> void {
    bool all_dldis{true};
    for (const auto source: additions) {
      if (source.type != dldi::SourceType::DynamicLinkedDataIndex) {
        all_dldis = false;
        break;
      }
    }

    if (!all_dldis) {
      throw std::runtime_error("This combination of sources is not yet supported");
    }

    std::vector<std::shared_ptr<dldi::DLDI>> add_dldis;
    std::vector<std::shared_ptr<dldi::DLDI>> rem_dldis;

    for (const auto source: additions) {
      add_dldis.push_back(std::make_shared<dldi::DLDI>(dldi::DLDI{source.path}));
    }
    for (const auto source: removals) {
      rem_dldis.push_back(std::make_shared<dldi::DLDI>(dldi::DLDI{source.path}));
    }

    const auto largest_subject_index{largest_dict_index(add_dldis, dldi::TripleTermPosition::subject)};
    const auto largest_predicate_index{largest_dict_index(add_dldis, dldi::TripleTermPosition::predicate)};
    const auto largest_object_index{largest_dict_index(add_dldis, dldi::TripleTermPosition::object)};

    merge_dictionaries(add_dldis, dldi::TripleTermPosition::subject, largest_subject_index);
    merge_dictionaries(add_dldis, dldi::TripleTermPosition::predicate, largest_predicate_index);
    merge_dictionaries(add_dldis, dldi::TripleTermPosition::object, largest_object_index);

    for (auto order: dldi::EnumMapping::TRIPLE_ORDERS) {
      merge_triples(add_dldis, rem_dldis, output_dir, largest_subject_index, largest_predicate_index, largest_object_index, order);
    }

    if (!rem_dldis.empty()) {
      apply_dict_removals(add_dldis.at(largest_subject_index), rem_dldis, dldi::TripleTermPosition::subject);
      apply_dict_removals(add_dldis.at(largest_predicate_index), rem_dldis, dldi::TripleTermPosition::predicate);
      apply_dict_removals(add_dldis.at(largest_object_index), rem_dldis, dldi::TripleTermPosition::object);
    }

    add_dldis.at(largest_subject_index)->get_dict(dldi::TripleTermPosition::subject)->save(dldi::Dictionary::dictionary_file_path(output_dir, dldi::TripleTermPosition::subject));
    add_dldis.at(largest_predicate_index)->get_dict(dldi::TripleTermPosition::predicate)->save(dldi::Dictionary::dictionary_file_path(output_dir, dldi::TripleTermPosition::predicate));
    add_dldis.at(largest_object_index)->get_dict(dldi::TripleTermPosition::object)->save(dldi::Dictionary::dictionary_file_path(output_dir, dldi::TripleTermPosition::object));
  }
}