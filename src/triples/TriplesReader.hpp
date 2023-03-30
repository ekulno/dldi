#ifndef DLDI_TRIPLES_READER_HPP
#define DLDI_TRIPLES_READER_HPP

#include <fcntl.h>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <sys/mman.h>
#include <vector>

#include <DLDI_enums.hpp>
#include <QuantifiedTriple.hpp>
#include <TriplesIterator.hpp>
#include <dictionary/Dictionary.hpp>

namespace dldi {
  using MmapInfo = std::tuple<int, const std::size_t*, const std::size_t>;

  class TriplesReader {
  public:
    TriplesReader(const std::filesystem::path& data_dir)
      : m_data_dir{data_dir} {};

    ~TriplesReader() {
      for (const auto& [_key, value]: primary_ids) {
        close(std::get<0>(value));
      }
      for (const auto& [_key, value]: secondary_ids) {
        close(std::get<0>(value));
      }
      for (const auto& [_key, value]: secondary_ref) {
        close(std::get<0>(value));
      }
      for (const auto& [_key, value]: tertiary_ids) {
        close(std::get<0>(value));
      }
      for (const auto& [_key, value]: tertiary_ref) {
        close(std::get<0>(value));
      }
    }

    auto open_mmap(const std::filesystem::path& path) -> MmapInfo {
      auto fd{open(path.c_str(), O_RDONLY)};
      if (fd == -1) {
        throw std::runtime_error("Failed to open file for reading " + path.string());
      }

      const auto filesize{std::filesystem::file_size(path)};
      const auto num_items{filesize / (sizeof(QuantifiedTriple))};

      auto ptr{reinterpret_cast<const std::size_t*>(mmap(0, filesize, PROT_READ, MAP_SHARED, fd, 0))};
      if (ptr == MAP_FAILED) {
        throw std::runtime_error("Failed to mmap triples file: " + path.string());
      };
      return {fd, ptr, num_items};
    }

    template <typename T>
    auto ensure_loaded(std::map<T, MmapInfo>& m, const T& key, std::string filename) -> void {
      if (m.find(key) == m.end()) {
        m.emplace(key, open_mmap(m_data_dir / filename));
      }
    }

    auto prepare_for_query(const dldi::TripleOrder& order) -> void {
      const auto [primary, _secondary, _tertiary] = EnumMapping::order_to_positions(order);
      ensure_loaded(primary_ids, primary, EnumMapping::position_to_string(primary) + ".primary-ids");
      ensure_loaded(secondary_ids, order, EnumMapping::order_to_string(order) + ".secondary-ids");
      ensure_loaded(secondary_ref, order, EnumMapping::order_to_string(order) + ".secondary-refs");
      ensure_loaded(tertiary_ids, order, EnumMapping::order_to_string(order) + ".tertiary-ids");
      ensure_loaded(tertiary_ref, order, EnumMapping::order_to_string(order) + ".tertiary-refs");
    }

    // auto query(const dldi::TriplePattern& pattern, const DictionariesHandle& dicts) -> TriplesIterator {
    //   return TriplesIterator{pattern, m_num_triples, dicts};
    // }
    auto latter_two(MmapInfo mmapInfo) const-> std::pair<const std::size_t*, const std::size_t>  {
      const auto [_,ptr,length] = mmapInfo;
      return {ptr,length};
    }
    auto search(const dldi::TriplePattern& pattern, const std::shared_ptr<DictionariesHandle>& dicts) const -> std::shared_ptr<TriplesIterator> {
      const auto order{QuantifiedTriple::decide_order_from_triple_pattern(pattern)};
      const auto [primary, _secondary, _tertiary]{EnumMapping::order_to_positions(order)};
      return std::make_shared<TriplesIterator>(
        pattern,
        order,
        dicts,
        latter_two(primary_ids.at(primary)),
        latter_two(secondary_ids.at(order)),
        latter_two(secondary_ref.at(order)),
        latter_two(tertiary_ids.at(order)),
        latter_two(tertiary_ref.at(order)));
    }

    auto num_triples() -> std::size_t {
      return m_num_triples;
    }

    // static auto triples_file_path(const std::filesystem::path& dldi_dir, const dldi::TripleOrder& order) -> std::filesystem::path {
    //   return dldi_dir.string() + "/" + EnumMapping::order_to_string(order) + ".triples";
    // }

    static auto validate_dir(const std::filesystem::path& dldi_dir) -> void {
      // for (auto order: EnumMapping::TRIPLE_ORDERS) {
      //   const auto filepath{triples_file_path(dldi_dir, order)};
      //   if (!std::filesystem::exists(filepath)) {
      //     throw std::runtime_error("Missing file " + filepath.string());
      //   }
      // }
    }

  private:
    int fd;
    std::size_t m_num_triples;
    const std::filesystem::path& m_data_dir;

    // fd, data-ptr, num-items
    std::map<dldi::TripleTermPosition, MmapInfo> primary_ids;
    std::map<dldi::TripleOrder, MmapInfo> secondary_ids;
    std::map<dldi::TripleOrder, MmapInfo> secondary_ref;
    std::map<dldi::TripleOrder, MmapInfo> tertiary_ids;
    std::map<dldi::TripleOrder, MmapInfo> tertiary_ref;
  };
}

#endif
