#ifndef DLDI_DICTIONARIES_HOLDER_HPP
#define DLDI_DICTIONARIES_HOLDER_HPP

#include <dictionary/Dictionary.hpp>

namespace dldi {

  class DictionariesHandle {
  public:

    // nb: this involves computation which isn't always possible to optimize away,
    // even when the human programmer knows which dict it'll be. 
    // so, sometimes it's better to use subjects(), predicates(), and objects(). 
    auto get_dict(const dldi::TripleTermPosition& position) const -> std::shared_ptr<dldi::Dictionary> {
      if (position == dldi::TripleTermPosition::subject) {
        return subjects();
      }
      if (position == dldi::TripleTermPosition::predicate) {
        return predicates();
      }
      return objects();
    }
    
    auto subjects() const -> std::shared_ptr<dldi::Dictionary> {
      return m_subjects;
    }
    auto predicates() const -> std::shared_ptr<dldi::Dictionary> {
      return m_predicates;
    }
    auto objects() const -> std::shared_ptr<dldi::Dictionary> {
      return m_objects;
    }

    auto ensure_loaded(const dldi::TripleTermPosition& position, const std::filesystem::path& datadir) -> void {
      if (get_dict(position)) {
        return;
      }

      const auto dict{std::make_shared<Dictionary>(dldi::Dictionary::dictionary_file_path(datadir, position))};

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

    auto save(const std::filesystem::path& output_path) -> void {
      // todo try parallelizing this
      get_dict(dldi::TripleTermPosition::subject)->save(Dictionary::dictionary_file_path(output_path, dldi::TripleTermPosition::subject));
      get_dict(dldi::TripleTermPosition::predicate)->save(Dictionary::dictionary_file_path(output_path, dldi::TripleTermPosition::predicate));
      get_dict(dldi::TripleTermPosition::object)->save(Dictionary::dictionary_file_path(output_path, dldi::TripleTermPosition::object));
    }

    auto add(const std::string& term, const std::size_t& quantity, const dldi::TripleTermPosition& position) -> std::size_t {
      return get_dict(position)->add(term, quantity);
    }

    DictionariesHandle()
      : m_subjects{std::make_shared<dldi::Dictionary>()},
        m_predicates{std::make_shared<dldi::Dictionary>()},
        m_objects{std::make_shared<dldi::Dictionary>()} {
    }

  private:
    std::shared_ptr<dldi::Dictionary> m_subjects;
    std::shared_ptr<dldi::Dictionary> m_predicates;
    std::shared_ptr<dldi::Dictionary> m_objects;
  };
}

#endif
