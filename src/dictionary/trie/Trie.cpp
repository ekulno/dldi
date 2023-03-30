#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <type_traits>

#include <dictionary/trie/TermStringIterator.hpp>
#include <dictionary/trie/Trie.hpp>

#include "./DataManager/DataManager.hpp"
#include "./TrieAlgorithm/TrieAlgorithm.hpp"
#include "./TrieNavigator.hpp"
#include "PathCharIterator.hpp"

/**
 * Trie-based CSD.
 */
namespace csd {

  Trie::Trie()
    : m_data{new DataManager()} {
  }

  auto Trie::insert(const std::string& rdfTerm, const std::size_t& occurrences) -> std::pair<std::size_t, bool> {
    if (rdfTerm.size() < 2) {
      throw std::runtime_error("The smallest possible RDF term is 2 characters.");
    }
    auto r{TrieAlgorithm::insert(m_data, rdfTerm, occurrences)};
    r.first = m_data->internalToExposedId(r.first);
    return r;
  }

  void Trie::addOccurrences(const std::size_t& id, const std::size_t& occurences) {
    m_data->get_leafNode(m_data->exposedToInternalId(id))->occurences += occurences;
  }

  auto Trie::remove(const std::size_t& id, const std::size_t& occurences) -> bool {
    if (id == 0) {
      throw std::runtime_error("Tried to remove id 0");
    }
    return TrieAlgorithm::remove(m_data, m_data->exposedToInternalId(id), occurences);
  }
  Trie::~Trie() {
    delete m_data;
  }

  auto Trie::string_to_id(const std::string& str) const -> std::size_t {
    try {
      auto r{TrieAlgorithm::string_to_id(m_data, str)};
      return r.first;
    } catch (StringNotFoundException const&) {
      return 0;
    }
  }

  auto Trie::id_to_string(const std::size_t& id) const -> const std::string {
    if (id == 0) {
      throw std::runtime_error("Invalid Id (id_to_string)");
    }
    const auto internal_id{m_data->exposedToInternalId(id)};
    return TrieAlgorithm::id_to_string(m_data, internal_id);
  }

  auto Trie::get_path(const std::size_t& exposedId) const -> TriePath {
    if (exposedId == 0) {
      throw std::runtime_error("Invalid Id (get_path)");
    }
    const auto internalId{m_data->exposedToInternalId(exposedId)};
    return TrieAlgorithm::extract_path(m_data, internalId);
  }
  auto Trie::compare(
    const std::size_t& exposedId1,
    const std::size_t& exposedId2,
    const TriePath& path1,
    const TriePath& path2,
    const Trie* const otherCsdTrie) const -> int {
    if (this == otherCsdTrie) {
      if (exposedId1 == exposedId2) {
        return 0;
      }
      auto i1 = path1.size() - 1;
      auto i2 = path2.size() - 1;
      while (true) {
        if (path1.at(i1).first != path2.at(i2).first) {
          const auto* const label1{m_data->get_label(path1.at(i1).first, static_cast<Edge*>(path1.at(i1).second))};
          const auto* const label2{m_data->get_label(path2.at(i2).first, static_cast<Edge*>(path2.at(i2).second))};
          return label1[0] - label2[0];
        }
        i1--;
        i2--;
      }
    }

    auto it1{PathCharIterator(m_data, path1)};
    auto it2{PathCharIterator(otherCsdTrie->getData(), path2)};

    while (true) {
      if (!it1.has_next()) {
        if (!it2.has_next()) {
          return 0;
        }
        return -1;
      }
      if (!it2.has_next()) {
        return 1;
      }
      const auto comparison{it1.next() - it2.next()};
      if (comparison != 0) {
        return comparison;
      }
    }
  }

  /**
   * Returns negative if lhs < rhs 
   * Returns positive if rhs < lhs 
   * Returns 0 if lhs == rhs 
  */
  auto Trie::compare(
    const std::size_t& exposedId1,
    const std::size_t& exposedId2) const -> int {
    if (exposedId1 == 0 || exposedId2 == 0) {
      throw std::runtime_error("Invalid ID");
    }
    if (exposedId1 == exposedId2) {
      return 0;
    }
    const auto path1{get_path(exposedId1)};
    const auto path2{get_path(exposedId2)};
    auto i1 = path1.size() - 1;
    auto i2 = path2.size() - 1;
    int result;
    while (true) {
      if (path1.at(i1).first != path2.at(i2).first) {
        const auto* const label1{m_data->get_label(path1.at(i1).first, static_cast<Edge*>(path1.at(i1).second))};
        const auto* const label2{m_data->get_label(path2.at(i2).first, static_cast<Edge*>(path2.at(i2).second))};
        result = label1[0] - label2[0];
        break;
      }
      i1--;
      i2--;
    }
    return result;
  }

  auto Trie::suggestions(const std::string& prefix) const -> std::shared_ptr<TermStringIterator> {
    return std::make_shared<TermStringIterator>(m_data, prefix);
  }

  auto Trie::getStats() const -> const TrieStats* const {
    return m_data->getStats();
  }

  auto Trie::getData() const -> const DataManager* const {
    return m_data;
  }

  /**
   * @brief Write to a file.
   *
   */
  void Trie::save(std::ostream& fp) {
    m_data->save(fp);
  }

  auto Trie::load(unsigned char* ptr) -> void {
    m_data->load(ptr);
  }

  void Trie::print(std::size_t id) const {
    const auto* const stats{m_data->getStats()};
    std::cout << "\n### Stats \n\n";
    std::cout << std::endl;
    std::cout << " #edges " << stats->numEdges << std::endl;
    std::cout << " #internals " << stats->numInternalNodes << std::endl;
    std::cout << " #leaves " << stats->numLeaves << std::endl;
    std::cout << " #label bytes " << stats->numLabelBytes << std::endl;
    std::cout << std::endl;
    std::cout << "\n### Graph \n\n";
    std::cout << std::endl;
    std::cout << "digraph G {" << std::endl;
    std::cout << "node [shape = rectangle,fontsize=6, ordering=out,fontname=monofont];" << std::endl;
    std::cout << "edge [fontname=monofont];" << std::endl;
    std::cout << std::endl;

    auto navigator{TrieNavigator(m_data, id)};

    while (navigator.mayGoDown() || navigator.mayGoRight()) {
      // Construct a string for the edge label in the diagram.
      // This includes the trie edge's (escaped and null-terminated) label,

      auto edgeLabelChars{std::vector<unsigned char>()};
      unsigned char* label = navigator.label();
      for (long unsigned int j = 0; j < navigator.edge()->labelLength; j++) { // NOLINT(altera-unroll-loops)
        if (label[j] == '\\') {
          // escape the escape symbol
          edgeLabelChars.push_back('\\');
          edgeLabelChars.push_back('\\');
        } else if (label[j] == '"') {
          // escape the quote
          edgeLabelChars.push_back('\\');
        }
        edgeLabelChars.push_back(label[j]);
      }
      if (navigator.edge()->outNodeIsLeaf) {
        // use $ to denote string termination.
        edgeLabelChars.push_back('$');
      } else {
        // add null-byte so our output isn't garbled.
        // (leaf-node in-edge labels already end with null-byte)
        edgeLabelChars.push_back('\0');
      }

      // the in-node's identifier
      std::cout << 'O' << navigator.edge()->inNodeId;
      std::cout << " -> ";
      // the out-node's identifier
      std::cout << (navigator.edge()->outNodeIsLeaf ? 'X' : 'O') << navigator.edge()->outNodeId;
      // the label
      std::cout << " [label=\""
                << "\\[" << navigator.edgeId() << "\\]";
      for (auto edgeLabelChar: edgeLabelChars) {
        std::cout << edgeLabelChar;
      }
      std::cout << "\"]" << std::endl;

      // recurse through the trie

      if (navigator.mayGoDown()) {
        navigator.goDown();
      }
      if (navigator.mayGoRight()) {
        navigator.goRight();
      }
    }
    std::cout << "}" << std::endl;
  }
}
