#ifndef DLDI_CLI_HPP
#define DLDI_CLI_HPP

#include <filesystem>
#include <getopt.h>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include <DLDI.hpp>
#include <DLDI_Composer.hpp>

namespace dldi {
  class DldiCli {
  public:
    auto static help_query() -> void;
    auto static query(int argc, char** argv) -> int;

    // query_terms is used for querying by triple-pattern.
    // supplying no pattern is equivalent to serializing the whole dataset to plaintext data.

    auto static help_query_triples() -> void;
    auto static query_triples(int argc, char** argv) -> int;

    // query_terms can be used for prefix queries over one or more positions.
    // prefix queries can also be empty, meaning it's possible to use this for
    // getting a stream over all terms.

    auto static help_query_terms() -> void;
    auto static query_terms(int argc, char** argv) -> int;

    // compose() is used for creating new dldis, either from
    // plain-text linked-data files, or from existing dldis.

    auto static help_compose() -> void;
    auto static compose(int argc, char** argv) -> int;
  };
}

#endif