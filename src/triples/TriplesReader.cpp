#include <execution>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>

#include <DLDI.hpp>

#include "./TriplesReader.hpp"

namespace dldi {

  TriplesReader::TriplesReader(const std::filesystem::path& path) {
    fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
      throw std::runtime_error("Failed to open file for reading " + path.string());
    }

    const auto filesize{std::filesystem::file_size(path)};
    m_num_triples = filesize / (sizeof(QuantifiedTriple));

    m_triples = reinterpret_cast<const QuantifiedTriple*>(mmap(0, filesize, PROT_READ, MAP_SHARED, fd, 0));
    if (m_triples == MAP_FAILED) {
      throw std::runtime_error("Failed to mmap triples file: " + path.string());
    };
  }

  TriplesReader::~TriplesReader() {
    close(fd);
  }

  auto TriplesReader::query(const dldi::TriplePattern& pattern, const Dictionary& subjects, const Dictionary& predicates, const Dictionary& objects) -> TriplesIterator {
    return TriplesIterator{m_triples, pattern, m_num_triples, subjects, predicates, objects};
  }
  auto TriplesReader::query_ptr(const dldi::TriplePattern& pattern, const Dictionary& subjects, const Dictionary& predicates, const Dictionary& objects) const -> std::shared_ptr<TriplesIterator> {
    return std::make_shared<TriplesIterator>(m_triples, pattern, m_num_triples, subjects, predicates, objects);
  }

  auto TriplesReader::num_triples() -> std::size_t {
    return m_num_triples;
  }
}
