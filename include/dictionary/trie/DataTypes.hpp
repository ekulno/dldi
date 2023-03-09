#ifndef CSD_DataTypes_HPP
#define CSD_DataTypes_HPP

#include <cstddef>

namespace csd {
  struct LeafNode {
    std::size_t inEdge;
    std::size_t occurences;
  } __attribute__((packed));

  struct InternalNode {
    std::size_t inEdge;
    std::size_t outEdgesOffset;
    unsigned char numOutEdges;
  } __attribute__((packed));

  struct Edge {
    bool outNodeIsLeaf;
    std::size_t outNodeId;
    std::size_t inNodeId;
    std::size_t labelLength;
    std::size_t labelOffset;
    bool deleted;
  } __attribute__((packed));

}
#endif
