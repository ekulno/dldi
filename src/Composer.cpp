#include <algorithm>
#include <memory>

#include "./BinaryStreamWriter.hpp"
#include "./Composer.hpp"
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

  struct DT {
    std::shared_ptr<dldi::TriplesIterator> iterator;
    std::shared_ptr<dldi::Dictionary> subjects;
    std::shared_ptr<dldi::Dictionary> predicates;
    std::shared_ptr<dldi::Dictionary> objects;
  };

  class IdTriplesIteratorComparator {
  public:
    IdTriplesIteratorComparator(const dldi::TripleOrder& order)
      : m_order{order} {
    }

    [[nodiscard]] auto operator()(const std::shared_ptr<DT> lhs, const std::shared_ptr<DT> rhs) -> bool {
      // true: lhs should preceede rhs
      // false: rhs should preceede lhs

      if (lhs->iterator->has_next() && !rhs->iterator->has_next())
        return true;
      if (!lhs->iterator->has_next() && rhs->iterator->has_next())
        return false;
      if (!lhs->iterator->has_next() && !rhs->iterator->has_next())
        return true;

      if (m_order == dldi::TripleOrder::SPO) {
        const auto subject_comparison{lhs->subjects->compare(lhs->iterator->read().subject(), rhs->iterator->read().subject(), rhs->subjects)};
        if (subject_comparison != 0)
          return subject_comparison < 0;
        const auto predicate_comparison{lhs->predicates->compare(lhs->iterator->read().predicate(), rhs->iterator->read().predicate(), rhs->predicates)};
        if (predicate_comparison != 0)
          return predicate_comparison < 0;
        const auto object_comparison{lhs->objects->compare(lhs->iterator->read().object(), rhs->iterator->read().object(), rhs->objects)};
        if (object_comparison != 0)
          return object_comparison < 0;
        return true;
      }
      if (m_order == dldi::TripleOrder::SOP) {
        const auto subject_comparison{lhs->subjects->compare(lhs->iterator->read().subject(), rhs->iterator->read().subject(), rhs->subjects)};
        if (subject_comparison != 0)
          return subject_comparison < 0;
        const auto object_comparison{lhs->objects->compare(lhs->iterator->read().object(), rhs->iterator->read().object(), rhs->objects)};
        if (object_comparison != 0)
          return object_comparison < 0;
        const auto predicate_comparison{lhs->predicates->compare(lhs->iterator->read().predicate(), rhs->iterator->read().predicate(), rhs->predicates)};
        if (predicate_comparison != 0)
          return predicate_comparison < 0;
        return true;
      }
      if (m_order == dldi::TripleOrder::POS) {
        const auto predicate_comparison{lhs->predicates->compare(lhs->iterator->read().predicate(), rhs->iterator->read().predicate(), rhs->predicates)};
        if (predicate_comparison != 0)
          return predicate_comparison < 0;
        const auto object_comparison{lhs->objects->compare(lhs->iterator->read().object(), rhs->iterator->read().object(), rhs->objects)};
        if (object_comparison != 0)
          return object_comparison < 0;
        const auto subject_comparison{lhs->subjects->compare(lhs->iterator->read().subject(), rhs->iterator->read().subject(), rhs->subjects)};
        if (subject_comparison != 0)
          return subject_comparison < 0;
        return true;
      }
      if (m_order == dldi::TripleOrder::PSO) {
        const auto predicate_comparison{lhs->predicates->compare(lhs->iterator->read().predicate(), rhs->iterator->read().predicate(), rhs->predicates)};
        if (predicate_comparison != 0)
          return predicate_comparison < 0;
        const auto subject_comparison{lhs->subjects->compare(lhs->iterator->read().subject(), rhs->iterator->read().subject(), rhs->subjects)};
        if (subject_comparison != 0)
          return subject_comparison < 0;
        const auto object_comparison{lhs->objects->compare(lhs->iterator->read().object(), rhs->iterator->read().object(), rhs->objects)};
        if (object_comparison != 0)
          return object_comparison < 0;
        return true;
      }
      if (m_order == dldi::TripleOrder::OSP) {
        const auto object_comparison{lhs->objects->compare(lhs->iterator->read().object(), rhs->iterator->read().object(), rhs->objects)};
        if (object_comparison != 0)
          return object_comparison < 0;
        const auto subject_comparison{lhs->subjects->compare(lhs->iterator->read().subject(), rhs->iterator->read().subject(), rhs->subjects)};
        if (subject_comparison != 0)
          return subject_comparison < 0;
        const auto predicate_comparison{lhs->predicates->compare(lhs->iterator->read().predicate(), rhs->iterator->read().predicate(), rhs->predicates)};
        if (predicate_comparison != 0)
          return predicate_comparison < 0;
        return true;
      }
      if (m_order == dldi::TripleOrder::OPS) {
        const auto object_comparison{lhs->objects->compare(lhs->iterator->read().object(), rhs->iterator->read().object(), rhs->objects)};
        if (object_comparison != 0)
          return object_comparison < 0;
        const auto predicate_comparison{lhs->predicates->compare(lhs->iterator->read().predicate(), rhs->iterator->read().predicate(), rhs->predicates)};
        if (predicate_comparison != 0)
          return predicate_comparison < 0;
        const auto subject_comparison{lhs->subjects->compare(lhs->iterator->read().subject(), rhs->iterator->read().subject(), rhs->subjects)};
        if (subject_comparison != 0)
          return subject_comparison < 0;
        return true;
      }
      throw std::runtime_error("Unrecognized order");
    }

  private:
    const dldi::TripleOrder m_order;
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

        auto val = std::make_shared<DT>(DT{
          query_iterator,
          dldi->get_dict(dldi::TripleTermPosition::subject),
          dldi->get_dict(dldi::TripleTermPosition::predicate),
          dldi->get_dict(dldi::TripleTermPosition::object)});
        m_dts.push_back(val);
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
      auto id_next{dt->iterator->read()};

      const dldi::QuantifiedTriple id_final{
        dt->subjects == m_mapto_subjects_dict ? id_next.subject() : (m_mapto_subjects_dict->string_to_id(dt->subjects->id_to_string(id_next.subject()))),
        dt->predicates == m_mapto_predicates_dict ? id_next.predicate() : (m_mapto_predicates_dict->string_to_id(dt->predicates->id_to_string(id_next.predicate()))),
        dt->objects == m_mapto_objects_dict ? id_next.object() : (m_mapto_objects_dict->string_to_id(dt->objects->id_to_string(id_next.object()))),
        id_next.quantity()};

      m_next = id_final;

      dt->iterator->proceed();
      if (!dt->iterator->has_next()) {
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
    dldi::StreamWriter<dldi::QuantifiedTriple> triples{output_path.string() + "/" + dldi::DLDI::get_triples_name(order) + ".triples"};
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

    for (auto order: {dldi::TripleOrder::SPO, dldi::TripleOrder::SOP, dldi::TripleOrder::PSO, dldi::TripleOrder::POS, dldi::TripleOrder::OPS, dldi::TripleOrder::OSP}) {
      merge_triples(add_dldis, rem_dldis, output_dir, largest_subject_index, largest_predicate_index, largest_object_index, order);
    }

    if (!rem_dldis.empty()) {
      apply_dict_removals(add_dldis.at(largest_subject_index), rem_dldis, dldi::TripleTermPosition::subject);
      apply_dict_removals(add_dldis.at(largest_predicate_index), rem_dldis, dldi::TripleTermPosition::predicate);
      apply_dict_removals(add_dldis.at(largest_object_index), rem_dldis, dldi::TripleTermPosition::object);
    }

    add_dldis.at(largest_subject_index)->get_dict(dldi::TripleTermPosition::subject)->save(output_dir.string() + "/" + dldi::DLDI::get_dict_name(dldi::TripleTermPosition::subject) + ".dictionary");
    add_dldis.at(largest_predicate_index)->get_dict(dldi::TripleTermPosition::predicate)->save(output_dir.string() + "/" + dldi::DLDI::get_dict_name(dldi::TripleTermPosition::predicate) + ".dictionary");
    add_dldis.at(largest_object_index)->get_dict(dldi::TripleTermPosition::object)->save(output_dir.string() + "/" + dldi::DLDI::get_dict_name(dldi::TripleTermPosition::object) + ".dictionary");
  }
}
