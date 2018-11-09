cmake_minimum_required(VERSION 3.10)
project(ENCRYPTO_utils LANGUAGES CXX)

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
find_package(Boost REQUIRED COMPONENTS system thread)


add_library(encrypto_utils
    src/${PROJECT_NAME}/cbitvector.cpp
    src/${PROJECT_NAME}/channel.cpp
    src/${PROJECT_NAME}/circular_queue.cpp
    src/${PROJECT_NAME}/codewords.cpp
    src/${PROJECT_NAME}/connection.cpp
    src/${PROJECT_NAME}/crypto/crypto.cpp
    src/${PROJECT_NAME}/crypto/dgk.cpp
    src/${PROJECT_NAME}/crypto/djn.cpp
    src/${PROJECT_NAME}/crypto/ecc-pk-crypto.cpp
    src/${PROJECT_NAME}/crypto/gmp-pk-crypto.cpp
    src/${PROJECT_NAME}/crypto/intrin_sequential_enc8.cpp
    src/${PROJECT_NAME}/crypto/TedKrovetzAesNiWrapperC.cpp
    src/${PROJECT_NAME}/graycode.cpp
    src/${PROJECT_NAME}/parse_options.cpp
    src/${PROJECT_NAME}/powmod.cpp
    src/${PROJECT_NAME}/rcvthread.cpp
    src/${PROJECT_NAME}/sndthread.cpp
    src/${PROJECT_NAME}/socket.cpp
    src/${PROJECT_NAME}/thread.cpp
    src/${PROJECT_NAME}/timer.cpp
    src/${PROJECT_NAME}/utils.cpp
)
add_library(ENCRYPTO_utils::encrypto_utils ALIAS encrypto_utils)

target_compile_features(encrypto_utils PUBLIC cxx_std_17)
target_compile_options(encrypto_utils PRIVATE "-Wall" "-Wextra")

get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
target_include_directories(encrypto_utils
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)


target_link_libraries(encrypto_utils
    PRIVATE MIRACL::miracl
    PRIVATE Boost::system Boost::thread
    PUBLIC GMP::GMP GMP::GMPXX
    PUBLIC OpenSSL::Crypto
)


install(TARGETS encrypto_utils
        EXPORT "${PROJECT_NAME}Targets"
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        INCLUDES DESTINATION include
)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/"
        DESTINATION include
        FILES_MATCHING PATTERN "*.h"
)
export(TARGETS encrypto_utils NAMESPACE "${PROJECT_NAME}::" FILE "${PROJECT_NAME}Targets.cmake")
install(EXPORT "${PROJECT_NAME}Targets"
        NAMESPACE "${PROJECT_NAME}::"
        DESTINATION "lib/cmake/${PROJECT_NAME}"
)


include(CMakePackageConfigHelpers)

configure_package_config_file("${CMAKE_CURRENT_LIST_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION "lib/cmake/${PROJECT_NAME}"
)

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindGMP.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindGMPXX.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    DESTINATION "lib/cmake/${PROJECT_NAME}"
)
