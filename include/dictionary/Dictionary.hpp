#ifndef DLDI_DICT_HPP
#define DLDI_DICT_HPP

#include <filesystem>
#include <string>
#include <vector>

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
    auto query(const std::string& prefix) const -> csd::TermStringIterator;
    auto add(const std::string& term, const std::size_t& quantity) -> std::size_t;
    auto remove(const std::string& term, const std::size_t& quantity) -> void;
    auto save(const std::filesystem::path& path) -> void;

    auto compare(const std::size_t& lhs, const std::size_t& rhs) const -> int;
    auto compare(const std::size_t& lhs, const std::size_t& rhs, const std::shared_ptr<dldi::Dictionary> rhs_dict) const -> int;

    auto size() const -> std::size_t;

  private:
    csd::Trie m_trie;
    unsigned char* m_mmap_ptr;
    int m_fd;
  };
}

#endif