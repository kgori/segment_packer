cmake_minimum_required(VERSION 3.10)
project(SegmentPacking)
set(CMAKE_CXX_STANDARD 23)

add_library(stacker STATIC ${CMAKE_SOURCE_DIR}/src/segment.cpp ${CMAKE_SOURCE_DIR}/src/segment_packer.cpp)
target_include_directories(stacker PUBLIC include)

add_executable(main main.cpp)
target_include_directories(main PUBLIC include)
target_link_libraries(main stacker)
