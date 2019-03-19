cmake_minimum_required(VERSION 3.12)
project(ENCRYPTO_utils LANGUAGES CXX)

if (CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
    message(FATAL_ERROR "ENCRYPTO_utils require at least g++-8")
endif()

option(ENCRYPTO_UTILS_BUILD_TESTS "Build tests" Off)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(MIRACL QUIET)
if(MIRACL_FOUND)
    message(STATUS "Found MIRACL")
elseif(NOT MIRACL_FOUND AND NOT TARGET MIRACL::miracl)
    message("MIRACL was not found: add MIRACL subdirectory")
    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/MIRACL/CMakeLists.txt")
        find_package(Git REQUIRED)
        message("initialize Git submodule: extern/MIRACL")
        execute_process(COMMAND git submodule update --init extern/MIRACL
                        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
    endif()
    add_subdirectory(extern/MIRACL)
endif()
find_package(GMP REQUIRED)
find_package(GMPXX REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost 1.66.0 REQUIRED COMPONENTS system thread)

add_subdirectory(src)

if(ENCRYPTO_UTILS_BUILD_TESTS)
	add_subdirectory(extern/googletest EXCLUDE_FROM_ALL)
	add_subdirectory(test)
endif()
