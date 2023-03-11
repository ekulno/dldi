# Copy all test resources into the build dir

file(GLOB files "data/*")
foreach(file ${files})
    string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} targetpath ${file})
    configure_file(${file} ${targetpath} COPYONLY)
endforeach()

find_package(Catch2 REQUIRED)
add_executable(UnitTests test.cpp)
target_link_libraries(UnitTests lib-dldi Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(UnitTests)
