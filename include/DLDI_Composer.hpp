#ifndef DLDI_COMPOSER_HPP
#define DLDI_COMPOSER_HPP

#include <cassert>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include <DLDI_enums.hpp>
#include <QuantifiedTriple.hpp>
#include <TriplesIterator.hpp>
#include <dictionary/DictionariesHandle.hpp>
#include <dictionary/Dictionary.hpp>
#include <dictionary/trie/Trie.hpp>

namespace dldi {

  class DLDI_Composer {
  public:
  
    /**
     * Compose a DLDI instance from sets of resources which should be added and subtracted.
    */
    static auto compose(
      const std::vector<std::filesystem::path>& additions,
      const std::vector<std::filesystem::path>& subtractions,
      const std::filesystem::path& output_path) -> void;

  private:
  
  };
}

#endif
