cmake_minimum_required(VERSION 3.13)

#Watch out when changing name of ENCRYPTO_utils_* variables: 
#They are referenced as ${PROJECT_NAME}_* in InstallConfig.cmake and ExternalBuildHelper.cmake
if(ANDROID)
     if(ANDROID_ARM_NEON)
         set(ENCRYPTO_utils_INSTALL_PREFIX "platforms/${ANDROID_PLATFORM}/arch-${ANDROID_SYSROOT_ABI}/NEON")
     else(ANDROID_ARM_NEON)
         set(ENCRYPTO_utils_INSTALL_PREFIX "platforms/${ANDROID_PLATFORM}/arch-${ANDROID_SYSROOT_ABI}")
     endif(ANDROID_ARM_NEON)
     set(ENCRYPTO_utils_INSTALL_INCLUDE "${ENCRYPTO_utils_INSTALL_PREFIX}/include")
     set(ENCRYPTO_utils_INSTALL_LIB "${ENCRYPTO_utils_INSTALL_PREFIX}/lib")
     set(ENCRYPTO_utils_INSTALL_ARCHIVE "${ENCRYPTO_utils_INSTALL_PREFIX}/lib")
     set(ENCRYPTO_utils_INSTALL_CONFIG "")
else(ANDROID)
     set(ENCRYPTO_utils_INSTALL_PREFIX "")
     set(ENCRYPTO_utils_INSTALL_INCLUDE "include")
     set(ENCRYPTO_utils_INSTALL_LIB "lib")
     set(ENCRYPTO_utils_INSTALL_ARCHIVE "lib")
     set(ENCRYPTO_utils_INSTALL_CONFIG "")
endif(ANDROID)

#Watch out when changing name of variable ENCRPYTO_utils_DEPENDENCIES_TO_EXPORT:
#It is referenced as ${PROJECT_NAME}_DEPENDENCIES_TO_EXPORT in AddBoost.cmake
set(ENCRYPTO_utils_DEPENDENCIES_TO_EXPORT)

if(ANDROID AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${ANDROID_NDK}/usr/local" 
        CACHE PATH "Default install directory for android builds." FORCE)
endif()

list(APPEND BOOST_TARGETS_REQUIRED Boost::system Boost::thread)

include(AddGMP)
include(AddBoost)
include(AddOpenSSL)
include(AddRelic)

add_library(encrypto_utils ${ENCRYPTO_utils_LIBRARY_TYPE}
    ${PROJECT_NAME}/cbitvector.cpp
    ${PROJECT_NAME}/channel.cpp
    ${PROJECT_NAME}/circular_queue.cpp
    ${PROJECT_NAME}/codewords.cpp
    ${PROJECT_NAME}/connection.cpp
    ${PROJECT_NAME}/crypto/crypto.cpp
    ${PROJECT_NAME}/crypto/dgk.cpp
    ${PROJECT_NAME}/crypto/djn.cpp
    ${PROJECT_NAME}/crypto/ecc-pk-crypto.cpp
    ${PROJECT_NAME}/crypto/gmp-pk-crypto.cpp
    ${PROJECT_NAME}/crypto/intrin_sequential_enc8.cpp
    ${PROJECT_NAME}/crypto/TedKrovetzAesNiWrapperC.cpp
    ${PROJECT_NAME}/graycode.cpp
    ${PROJECT_NAME}/parse_options.cpp
    ${PROJECT_NAME}/powmod.cpp
    ${PROJECT_NAME}/rcvthread.cpp
    ${PROJECT_NAME}/sndthread.cpp
    ${PROJECT_NAME}/socket.cpp
    ${PROJECT_NAME}/thread.cpp
    ${PROJECT_NAME}/timer.cpp
    ${PROJECT_NAME}/utils.cpp
)
set_target_properties(encrypto_utils PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(ENCRYPTO_utils::encrypto_utils ALIAS encrypto_utils)

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/ENCRYPTO_utils/cmake_constants.h.in"
    "${PROJECT_BINARY_DIR}/include/cmake_constants.h"
)

target_compile_options(encrypto_utils PRIVATE "-Wall" "-Wextra")

get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
target_include_directories(encrypto_utils
    PUBLIC
        $<INSTALL_INTERFACE:${ENCRYPTO_utils_INSTALL_INCLUDE}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        
)

target_link_libraries(encrypto_utils
    PUBLIC Relic::relic
    PUBLIC GMP::GMP 
    PUBLIC GMP::GMPXX
    PUBLIC OpenSSL::Crypto
    PRIVATE Boost::system 
    PRIVATE Boost::thread
)

include(InstallConfig)
install_config(ENCRYPTO_utils_INSTALL_LOCATION "ENCRYPTO_utils" ENCRYPTO_utils::encrypto_utils)

install(TARGETS encrypto_utils ${ENCRYPTO_utils_DEPENDENCIES_TO_EXPORT}
        EXPORT "${PROJECT_NAME}Targets"
        ARCHIVE DESTINATION "${ENCRYPTO_utils_INSTALL_ARCHIVE}"
        LIBRARY DESTINATION "${ENCRYPTO_utils_INSTALL_LIB}"
        INCLUDES DESTINATION "${ENCRYPTO_utils_INSTALL_INCLUDE}")
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
        DESTINATION "${ENCRYPTO_utils_INSTALL_INCLUDE}"
        FILES_MATCHING REGEX ".*\\.h(pp|h)?$" REGEX ".*cmake.*" EXCLUDE)
install(FILES "${PROJECT_BINARY_DIR}/include/cmake_constants.h"
        DESTINATION "${ENCRYPTO_utils_INSTALL_INCLUDE}")
export(TARGETS encrypto_utils ${ENCRYPTO_utils_DEPENDENCIES_TO_EXPORT}
       NAMESPACE "${PROJECT_NAME}::"
       FILE "${PROJECT_NAME}Targets.cmake")
install(EXPORT "${PROJECT_NAME}Targets"
        NAMESPACE "${PROJECT_NAME}::"
        DESTINATION "${ENCRYPTO_utils_INSTALL_LOCATION}")
