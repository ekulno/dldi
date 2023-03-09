#ifndef RDF_LIBZ_SERD_STREAM_HPP
#define RDF_LIBZ_SERD_STREAM_HPP

#include <cstddef>
#include <filesystem>

#include <zlib.h>

namespace rdf {
  class LibzSerdStream {
  public:
    explicit LibzSerdStream(const std::filesystem::path& file)
      : m_file{gzopen(file.c_str(), "rb")} {
      if (!m_file) {
        throw std::runtime_error("Could not open input file for parsing.");
      }
    }
    ~LibzSerdStream() {
      gzclose(m_file);
    }
    static auto error(void* stream) -> int {
      LibzSerdStream* l_in = reinterpret_cast<LibzSerdStream*>(stream);
      return l_in->m_err;
    }
    static auto read(void* buffer, std::size_t size, std::size_t nmemb, void* stream) -> std::size_t {
      LibzSerdStream* l_in = reinterpret_cast<LibzSerdStream*>(stream);
      const int numRead{gzread(l_in->m_file, buffer, nmemb * size)};
      if (numRead == -1) {
        l_in->m_err = 1;
      }
      return numRead;
    }

  private:
    int m_err{0};
    gzFile m_file;
  };
}

#endif
