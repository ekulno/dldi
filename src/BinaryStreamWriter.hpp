#ifndef DLDI_STREAM_WRITER_HPP
#define DLDI_STREAM_WRITER_HPP

#include <filesystem>
#include <fstream>

namespace dldi {
  template <typename T>
  class StreamWriter {
  public:
    StreamWriter(const std::filesystem::path& outpath)
      : m_out{outpath, std::ios::binary | std::ios::trunc} {
      if (!m_out.good()) {
        throw std::runtime_error("Error opening file to save data: " + outpath.string());
      }
    }
    ~StreamWriter() {
      if (m_out) {
        m_out.close();
      }
    };
    auto write(const T& data) -> void {
      m_out.write(reinterpret_cast<const char*>(&data), sizeof(data));
    }

  private:
    std::ofstream m_out;
  };
}

#endif