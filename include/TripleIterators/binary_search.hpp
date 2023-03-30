#ifndef DLDI_BINARY_SEARCH_HPP
#define DLDI_BINARY_SEARCH_HPP

#include <QuantifiedTriple.hpp>
#include <TripleIterators/binary_search.hpp>

namespace dldi {
  class BinarySearch {
  public:
    /**
     * Find the index of an id within an array, by using binary search. 
     * The array items are assumed to be dictionary IDs,
     * and the array is assumed to be sorted lexicographically by the dereferenced terms.  
    */
    static auto binary_search(const std::size_t* ids, const std::size_t& min,const std::size_t& max, const std::size_t id, std::shared_ptr<dldi::Dictionary> dict) -> std::size_t {
      std::size_t lower_bound{min};
      std::size_t upper_bound{max};

      while (true) {
        const std::size_t i{((upper_bound - lower_bound) / 2) + lower_bound};

        const auto comparison{dict->compare(id, ids[i])};
        if (comparison == 0) {
          return i;
        }
        if (comparison < 0) {
          // id is further to the left. cut search space in half, with i barely out-of-range.
          upper_bound = i - 1;
        }
        // id is further to the right. cut search space in half, with i barely out-of-range
        lower_bound = i + 1;
      }
    }
  };
}
#endif