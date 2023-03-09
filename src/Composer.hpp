#ifndef ZIPPER_HPP
#define ZIPPER_HPP

#include <DLDI.hpp>

namespace dldi {

  using SourceInfoVector = const std::vector<dldi::SourceInfo>;

  class Composer {
  public:
    Composer() = default;

    /**
     * Takes a set of sources which should be added or subtracted, 
     * and writes a result DLDI to the specified output dir. 
     * 
     * For the procedure to succeed, the following must hold: 
     *  - The number of subtractions of a triple or a term 
     *    must not exceed its number of additions.   
    */
    auto zip(SourceInfoVector& additions, SourceInfoVector& removals, const std::filesystem::path& output_dir) -> void;

  private:
    // auto merge_dictionary(const dldi::TripleTermPosition& position, SourceInfoVector& additions, SourceInfoVector& removals) -> void;
  };
}

#endif