cmake_minimum_required(VERSION 3.13)

if(NOT OpenSSL_SOURCE STREQUAL "")
    set(OpenSSL_DOWNLOAD_COMMANDS URL "${OpenSSL_SOURCE}")
else()
    set(OpenSSL_DOWNLOAD_COMMANDS URL "${OpenSSL_URL}" 
                                  URL_HASH "${OpenSSL_URL_HASH}")
    if(NOT "${DEPENDENCY_DIR}" STREQUAL "")
        list(APPEND OpenSSL_DOWNLOAD_COMMANDS DOWNLOAD_DIR ${DEPENDENCY_DIR})
    endif()
endif()

include(ExternalBuildHelper)
find_program(PERL_EXE NAMES perl)
find_program(MAKE_EXE NAMES gmake nmake make)

if(ANDROID)
    #set path to android build tools
    set(OpenSSL_PATH "${ANDROID_NDK}/toolchains/llvm/prebuilt/${ANDROID_HOST_TAG}/bin")
    string(APPEND OpenSSL_PATH ":${ANDROID_NDK}/toolchains/arm-linux-androideabi-4.9/prebuilt/${ANDROID_HOST_TAG}/bin")
    string(APPEND OpenSSL_PATH ":$ENV{PATH}")
    get_android_target_compiler(OpenSSL_ANDROID_COMPILER ${ANDROID_ABI})
    #set environment variables for android builds
    set(OpenSSL_ENVIRONMENT_VARIABLES
        "CC=clang"
        "CFLAGS=-target ${OpenSSL_ANDROID_COMPILER}"
        "CXX=clang++"
        "CXXFLAGS=-target ${OpenSSL_ANDROID_COMPILER}"
        #some openssl versions have ANDROID_NDK_HOME, some have ANDROID_NDK
        "ANDROID_NDK_HOME=${ANDROID_NDK}"
        "ANDROID_NDK=${ANDROID_NDK}"
        "PATH=${OpenSSL_PATH}"
    )
    set(OpenSSL_CONFIGURE_COMMAND_LINE 
        "${CMAKE_COMMAND}" -E env ${OpenSSL_ENVIRONMENT_VARIABLES}
        ${PERL_EXE} <SOURCE_DIR>/Configure android-${ANDROID_ARCH_NAME} -D__ANDROID_API__=${ANDROID_PLATFORM_LEVEL}
                               --prefix=<INSTALL_DIR> --openssldir=<INSTALL_DIR> --release)
else()
    set(OpenSSL_CONFIGURE_COMMAND_LINE 
        "${CMAKE_COMMAND}" -E env ${OpenSSL_ENVIRONMENT_VARIABLES} 
        ${PERL_EXE} <SOURCE_DIR>/config --prefix=<INSTALL_DIR> --openssldir=<INSTALL_DIR> --release)
endif()
set(OpenSSL_BUILD_COMMAND_LINE "${CMAKE_COMMAND}" -E env ${OpenSSL_ENVIRONMENT_VARIABLES} ${MAKE_EXE})
#install_sw will prevent openssl docs to be installed.
set(OpenSSL_INSTALL_COMMAND_LINE "${CMAKE_COMMAND}" -E env ${OpenSSL_ENVIRONMENT_VARIABLES} ${MAKE_EXE} install_sw)

include(ExternalProject)

ExternalProject_Add(OpenSSL
    PREFIX OpenSSL_PREFIX
    ${OpenSSL_DOWNLOAD_COMMANDS}
    UPDATE_DISCONNECTED TRUE
    CONFIGURE_COMMAND ${OpenSSL_CONFIGURE_COMMAND_LINE}
    BUILD_COMMAND ${OpenSSL_BUILD_COMMAND_LINE}
    INSTALL_COMMAND ${OpenSSL_INSTALL_COMMAND_LINE})

add_imported_library(TARGET OpenSSL::SSL ${OpenSSL_LIBRARY_TYPE}
                     EXTERNAL_TARGET OpenSSL
                     EXTERNAL_LIB_DIR "lib"
                     EXTERNAL_LIB_NAME ssl
                     EXTERNAL_INCLUDES "include")
add_imported_library(TARGET OpenSSL::Crypto ${OpenSSL_LIBRARY_TYPE}
                     EXTERNAL_TARGET OpenSSL
                     EXTERNAL_LIB_DIR "lib"
                     EXTERNAL_LIB_NAME crypto
                     EXTERNAL_INCLUDES "include")
