
#ifndef CSD_DataManager_utils_HPP
#define CSD_DataManager_utils_HPP

#include <stdexcept>
#include <vector>

#include "DataManager.hpp"

#define CAPACITY_GROWTH_FACTOR 2

namespace csd {

  template <class T>
  extern auto get_item(const TypedMmapPointer<T>* const mmap, const TrieBuffer<T>* const buffer, std::size_t id) -> T* const {
    if (id < mmap->length) {
      return &mmap->ptr[id];
    }
    return &(buffer->buf[id - mmap->length]);
  }

  inline auto add_hole_sequential(std::size_t id, std::vector<csd::Hole>& holes) -> void {
    if (holes.empty()) {
      struct Hole newHole {
        .start = id,
        .size = 1,
        .cumulative = 1
      };
      holes.push_back(newHole);
      return;
    }

    if (holes.at(holes.size() - 1).start + holes.at(holes.size() - 1).size == id) {
      holes.at(holes.size() - 1).size += 1;
      holes.at(holes.size() - 1).cumulative += 1;
      return;
    }

    struct Hole newHole {
      .start = id,
      .size = 1,
      .cumulative = holes.at(holes.size() - 1).cumulative + 1
    };
    holes.push_back(newHole);
  }

  template <class T>
  extern void possibly_realloc(TrieBuffer<T>* tb) {
    // this assumes we always add in batches of 1
    if (tb->length < tb->capacity) {
      return;
    }
    tb->capacity *= CAPACITY_GROWTH_FACTOR;
    tb->buf = (T*)(realloc(tb->buf, tb->capacity * sizeof(T)));
  }
}
#endif
