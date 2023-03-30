project(DLDI)

cmake_minimum_required(VERSION 3.25.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_library(lib-dldi)

include(GNUInstallDirs)

set(CMAKE_INSTALL_DOCDIR ${CMAKE_INSTALL_DATAROOTDIR}/doc/${PROJECT_NAME})
set(CXX_EXTENSIONS OFF)
set(CXX_STANDARD 26)
set(CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")


  include(FindPkgConfig)

  function(pkg_import target id pkg)
    if(NOT TARGET ${target})
      pkg_check_modules(${id} REQUIRED IMPORTED_TARGET ${pkg})
      add_library(${target} ALIAS PkgConfig::${id})
    endif()
  endfunction()

  pkg_import(serd::serd serd serd-0)

find_package(serd-o QUIET)

set_property(TARGET lib-dldi PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(lib-dldi
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>)

find_package(ZLIB REQUIRED)
target_link_libraries(lib-dldi
  PUBLIC
  ZLIB::ZLIB
    serd::serd)
target_sources(lib-dldi
  PRIVATE
    src/DLDI.cpp
    src/DLDI_compose.cpp

    src/triples/TriplesWriter.cpp

    src/dictionary/Dictionary.cpp

    src/dictionary/trie/Trie.cpp

    src/dictionary/trie/DataManager/DataManager.cpp
    src/dictionary/trie/DataManager/edges.cpp
    src/dictionary/trie/DataManager/internal-nodes.cpp
    src/dictionary/trie/DataManager/edge-labels.cpp
    src/dictionary/trie/DataManager/leaf-nodes.cpp
    src/dictionary/trie/DataManager/load.cpp
    src/dictionary/trie/DataManager/out-edges.cpp
    src/dictionary/trie/DataManager/save.cpp

    src/dictionary/trie/LabelComparator.cpp

    src/dictionary/trie/OutEdgeIterator/wrapper.cpp
    src/dictionary/trie/OutEdgeIterator/mmapped.cpp
    src/dictionary/trie/OutEdgeIterator/memory.cpp

    src/dictionary/trie/PathCharIterator.cpp
    src/dictionary/trie/TermIterator.cpp
    
    src/dictionary/trie/TrieAlgorithm/id_to_string.cpp
    src/dictionary/trie/TrieAlgorithm/insert.cpp
    src/dictionary/trie/TrieAlgorithm/string_to_id.cpp
    src/dictionary/trie/TrieAlgorithm/remove.cpp
    src/dictionary/trie/TrieAlgorithm/scope.cpp
    
    src/dictionary/trie/TrieNavigator.cpp
    src/dictionary/trie/TermStringIterator.cpp

    src/AnyPositionTermIterator.cpp

    src/rdf/SerdParser.cpp

    src/Composer.cpp
    )

add_executable(dldi)
target_link_libraries(dldi PRIVATE lib-dldi)
target_sources(dldi PRIVATE 
  src/cli/compose.cpp
  src/cli/query.cpp
  src/cli/query-terms.cpp
  src/cli/query-triples.cpp
  src/cli/main.cpp
  )
install(TARGETS dldi)

install(TARGETS lib-dldi
  EXPORT ${PROJECT_NAME}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.hpp")

enable_testing()

add_subdirectory(test)
