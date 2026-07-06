cmake_minimum_required(VERSION 3.10)
project(SegmentPacking)
set(CMAKE_CXX_STANDARD 23)

add_library(stacker STATIC ${CMAKE_SOURCE_DIR}/src/segment.cpp ${CMAKE_SOURCE_DIR}/src/segment_packer.cpp)
target_include_directories(stacker PUBLIC include)

add_executable(main main.cpp)
target_include_directories(main PUBLIC include)
target_link_libraries(main stacker)


find_program(R_EXECUTABLE R)
if(R_EXECUTABLE)
    message(STATUS "R found: ${R_EXECUTABLE}. Enabling R wrapper target.")

    # 1. Automatically generate the RcppExports.cpp / wrapper functions
    add_custom_target(r_attributes
        COMMAND ${R_EXECUTABLE} -e "Rcpp::compileAttributes('${CMAKE_CURRENT_SOURCE_DIR}/r-package'); if(requireNamespace('roxygen2', quietly=TRUE)) { roxygen2::roxygenize('${CMAKE_CURRENT_SOURCE_DIR}/r-package') } else { message('Warning: roxygen2 not found. Skipping NAMESPACE/documentation auto-generation.') }"
        VERBATIM
        COMMENT "Generating Rcpp attributes..."
    )

    # 2. Target to build/install the R package locally
    add_custom_target(r_package
        DEPENDS r_attributes
        COMMAND ${R_EXECUTABLE} CMD INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/r-package --clean
        COMMENT "Building and installing R package..."
    )
else()
    message(WARNING "R executable not found. R wrapper target will not be available.")
endif()
