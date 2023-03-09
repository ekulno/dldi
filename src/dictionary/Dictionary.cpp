#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>

#include <DLDI.hpp>

#include "./trie/TrieAlgorithm/TrieAlgorithm.hpp"
#include <dictionary/Dictionary.hpp>

namespace dldi {
  Dictionary::Dictionary(const std::filesystem::path& path) {
    m_fd = open(path.c_str(), O_RDONLY);
    if (m_fd == -1) {
      throw std::runtime_error("Failed to open file for reading " + path.string());
    }

    const auto filesize{std::filesystem::file_size(path)};
    m_mmap_ptr = reinterpret_cast<unsigned char*>(
      mmap(
        0,
        filesize,
        // we need write-permissions because we will sometimes write directly to memory-mapped area on updates,
        // when this is possible due to pre- and post- serializations are of equal size.
        PROT_WRITE,
        MAP_PRIVATE,
        m_fd,
        0));
    if (m_mmap_ptr == MAP_FAILED) {
      throw std::runtime_error("Failed to mmap dictionary file: " + path.string());
    };

    m_trie.load(m_mmap_ptr);
  }

  Dictionary::~Dictionary() {
    close(m_fd);
  }

  auto Dictionary::string_to_id(const std::string& string) const -> std::size_t {
    return m_trie.string_to_id(string);
  }
  auto Dictionary::id_to_string(const std::size_t& id) const -> std::string {
    return m_trie.id_to_string(id);
  }
  auto Dictionary::query(const std::string& prefix) const -> csd::TermStringIterator {
    return m_trie.suggestions(prefix);
  }
  auto Dictionary::add(const std::string& term, const std::size_t& quantity) -> std::size_t {
    const auto result{m_trie.insert(term, quantity)};
    return result.first;
  }
  auto Dictionary::remove(const std::string& term, const std::size_t& quantity) -> void {
    const auto id{m_trie.string_to_id(term)};
    if (id == 0) {
      std::cout << "Term `" << term << "` , removing #" << quantity << " , id=" << id << std::endl;
      throw std::runtime_error("Tried to remove a term that's not present");
    }
    m_trie.remove(id, quantity);
  }
  auto Dictionary::save(const std::filesystem::path& path) -> void {
    std::ofstream out{path, std::ios::binary | std::ios::trunc};
    if (!out.good()) {
      throw std::runtime_error("Error opening file `" + path.string() + "`to save dictionary");
    }
    m_trie.save(out);
    out.close();
  }
  auto Dictionary::compare(const std::size_t& lhs, const std::size_t& rhs) const -> int {
    return m_trie.compare(lhs, rhs);
  }

  auto Dictionary::compare(const std::size_t& lhs, const std::size_t& rhs, const std::shared_ptr<dldi::Dictionary> rhs_dict) const -> int {
    const auto lhs_tp{csd::TrieAlgorithm::extract_path(m_trie.getData(), lhs - 1)};
    const auto rhs_tp{csd::TrieAlgorithm::extract_path(rhs_dict->m_trie.getData(), rhs - 1)};
    return m_trie.compare(lhs, rhs, lhs_tp, rhs_tp, &(rhs_dict->m_trie));
  }

  auto Dictionary::size() const -> std::size_t {
    return m_trie.getStats()->numLeaves;
  }
}
