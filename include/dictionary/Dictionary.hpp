#ifndef DLDI_DICT_HPP
#define DLDI_DICT_HPP

#include <filesystem>
#include <string>
#include <vector>

#include <DLDI_enums.hpp>

#include <dictionary/trie/TermStringIterator.hpp>
#include <dictionary/trie/Trie.hpp>

namespace dldi {
  class Dictionary {
  public:
    Dictionary(const std::filesystem::path& path);
    Dictionary() = default;
    ~Dictionary();
    auto string_to_id(const std::string& string) const -> std::size_t;
    auto id_to_string(const std::size_t& id) const -> std::string;
    auto query(const std::string& prefix) const -> std::shared_ptr<csd::TermStringIterator>;
    auto add(const std::string& term, const std::size_t& quantity) -> std::size_t;
    auto remove(const std::string& term, const std::size_t& quantity) -> void;
    auto save(const std::filesystem::path& path) -> void;

    auto compare(const std::size_t& lhs, const std::size_t& rhs) const -> int;
    auto compare(const std::size_t& lhs, const std::size_t& rhs, const std::shared_ptr<dldi::Dictionary> rhs_dict) const -> int;

    auto size() const -> std::size_t;

    static auto dictionary_file_path(const std::filesystem::path& dldi_dir, const dldi::TripleTermPosition& position) -> std::filesystem::path {
      return dldi_dir.string() + "/" + EnumMapping::position_to_string(position) + "s.dictionary";
    }

    static auto validate_dir(const std::filesystem::path& dldi_dir) -> void {
      for ( auto position: {dldi::TripleTermPosition::subject, dldi::TripleTermPosition::predicate, dldi::TripleTermPosition::object}) {
        const auto filepath{ dictionary_file_path(dldi_dir, position)};
        if (!std::filesystem::exists(filepath)) {
          throw std::runtime_error("Missing file " + filepath.string());
        }
      }
    }

  private:
    csd::Trie m_trie;
    unsigned char* m_mmap_ptr;
    int m_fd;
  };
}

#endif