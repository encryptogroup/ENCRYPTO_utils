cmake_minimum_required(VERSION 3.13)

if(NOT ${GMP_SOURCE} STREQUAL "")
    set(GMP_LOCATION ${GMP_SOURCE})
    set(GMP_URL_HASH_COMMAND)
else()
    set(GMP_LOCATION ${GMP_URL})
    set(GMP_URL_HASH_COMMAND URL_HASH ${GMP_URL_HASH})
endif()

#Set compiler prefix for android build
if("${ANDROID_ABI}" STREQUAL "armeabi-v7a" OR "${ANDROID_ABI}" STREQUAL "armeabi-v7a with NEON")
    set(GMP_COMPILER_PREFIX "${ANDROID_TOOLCHAIN_ROOT}/bin/armv7a-linux-androideabi${ANDROID_PLATFORM_LEVEL}-")
elseif("${ANDROID_ABI}" STREQUAL "arm64-v8a")
    set(GMP_COMPILER_PREFIX "${ANDROID_TOOLCHAIN_ROOT}/bin/aarch64-linux-android${ANDROID_PLATFORM_LEVEL}-")
elseif("${ANDROID_ABI}" STREQUAL "x86")
    set(GMP_COMPILER_PREFIX "${ANDROID_TOOLCHAIN_ROOT}/bin/i686-linux-android${ANDROID_PLATFORM_LEVEL}-")
elseif("${ANDROID_ABI}" STREQUAL "x86_64")
    set(GMP_COMPILER_PREFIX "${ANDROID_TOOLCHAIN_ROOT}/bin/x86_64-linux-android${ANDROID_PLATFORM_LEVEL}-")
endif()

#If dependency dir is not empty, download into dependency dir, to prevent that subsequent builds to redownload gmp source. 
if(NOT "${DEPENDENCY_DIR}" STREQUAL "")
    set(GMP_DOWNLOAD_DIR_COMMAND DOWNLOAD_DIR ${DEPENDENCY_DIR})
else()
    set(GMP_DOWNLOAD_DIR_COMMAND "")
endif()
set(GMP_DOWNLOAD_PREFIX GMP_PREFIX)
set(GMP_DOWNLOAD_TARGET_NAME GMP)
set(GMP_C_FLAGS ${CMAKE_C_FLAGS})
set(GMP_CXX_FLAGS ${CMAKE_CXX_FLAGS})
string(TOLOWER "${GMP_LIBRARY_TYPE}" GMP_LIBRARY_TYPE_LOWER)

macro(add_gmp_flags flags)
    string(APPEND ${flags} " -Wno-unused-command-line-argument")
    string(APPEND ${flags} " -Wno-unused-value")
    string(APPEND ${flags} " -Wno-shift-op-parentheses")
    string(APPEND ${flags} " -fPIC")
endmacro(add_gmp_flags)

add_gmp_flags(GMP_C_FLAGS)
if(NOT GMP_ONLY)
    add_gmp_flags(GMP_CXX_FLAGS)
endif()
if(ANDROID)
    set(GMP_ENVIRONMENT_VARIABLES
        "CC=${GMP_COMPILER_PREFIX}clang"
        "CFLAGS=${GMP_C_FLAGS}"
        "LDFLAGS=${CMAKE_SHARED_LINKER_FLAGS}"
    )
    if(NOT GMP_ONLY)
        set(GMP_ENVIRONMENT_VARIABLES
            ${GMP_ENVIRONMENT_VARIABLES}
            "CXX=${GMP_COMPILER_PREFIX}clang++"
            "CXXFLAGS=${GMP_CXX_FLAGS}"
        )
    endif()
    set(GMP_CONFIGURE_FLAGS 
        --prefix=<INSTALL_DIR>
        --enable-static
        --enable-shared
        --host=${CMAKE_C_COMPILER_TARGET}
         ${GMP_ENVIRONMENT_VARIABLES}
    )
else(ANDROID)
    set(GMP_ENVIRONMENT_VARIABLES "CFLAGS=${GMP_C_FLAGS}")
    if(NOT GMP_ONLY)
        set(GMP_ENVIRONMENT_VARIABLES
            ${GMP_ENVIRONMENT_VARIABLES}
            "CXXFLAGS=${GMP_CXX_FLAGS}"
        )
    endif()
    set(GMP_CONFIGURE_FLAGS
        --prefix=<INSTALL_DIR>
        --enable-static
        --enable-shared
        ${GMP_ENVIRONMENT_VARIABLES}
    )
endif(ANDROID)
if(NOT GMP_ONLY)
    set(GMP_CONFIGURE_FLAGS
        --enable-cxx
        ${GMP_CONFIGURE_FLAGS}
    )
endif()
if(ANDROID)
    set(GMP_CONFIGURE_COMMAND
        <SOURCE_DIR>/configure ${GMP_CONFIGURE_FLAGS}
    )
else()
    set(GMP_CONFIGURE_COMMAND <SOURCE_DIR>/configure ${GMP_CONFIGURE_FLAGS})
endif()
find_program(MAKE_EXE NAMES gmake nmake make)
set(GMP_BUILD_COMMAND ${MAKE_EXE} ${GMP_ENVIRONMENT_VARIABLES})
set(GMP_INSTALL_COMMAND ${MAKE_EXE} install ${GMP_ENVIRONMENT_VARIABLES})

include(ExternalProject)
ExternalProject_Add(${GMP_DOWNLOAD_TARGET_NAME}
    PREFIX ${GMP_DOWNLOAD_PREFIX}
    URL ${GMP_LOCATION}
    ${GMP_URL_HASH_COMMAND}
    ${GMP_DOWNLOAD_DIR_COMMAND}
    UPDATE_DISCONNECTED TRUE
    CONFIGURE_COMMAND ${GMP_CONFIGURE_COMMAND}
    BUILD_COMMAND ${GMP_BUILD_COMMAND}
    INSTALL_COMMAND ${GMP_INSTALL_COMMAND}
)
ExternalProject_Get_Property(${GMP_DOWNLOAD_TARGET_NAME} INSTALL_DIR)
ExternalProject_Get_Property(${GMP_DOWNLOAD_TARGET_NAME} BINARY_DIR)

#Path that will be created upon downloading the GMP library
set(GMP_INCLUDE_DIR "${INSTALL_DIR}/include")
set(GMP_LIB_DIR "${INSTALL_DIR}/lib")
#If path doesn't exist create it, to prevent an error.
#Path will be created by ExternalProject during Build step.
if(NOT EXISTS "${GMP_INCLUDE_DIR}")
    file(MAKE_DIRECTORY ${GMP_INCLUDE_DIR})
endif(NOT EXISTS "${GMP_INCLUDE_DIR}")
if(NOT EXISTS "${GMP_LIB_DIR}")
    file(MAKE_DIRECTORY "${GMP_LIB_DIR}")
endif(NOT EXISTS "${GMP_LIB_DIR}")

function(get_lib_file_name res lib)
    set(RES "${lib}")
    string(PREPEND RES "${CMAKE_${GMP_LIBRARY_TYPE}_LIBRARY_PREFIX}")
    string(APPEND RES "${CMAKE_${GMP_LIBRARY_TYPE}_LIBRARY_SUFFIX}")
    set(${res} "${RES}" PARENT_SCOPE)
endfunction(get_lib_file_name)

function(add_global_imported_target target_name lib_name)
    add_library(${target_name} ${GMP_LIBRARY_TYPE} IMPORTED GLOBAL)
    add_dependencies(${target_name} ${GMP_DOWNLOAD_TARGET_NAME})
    target_include_directories(${target_name} INTERFACE ${GMP_INCLUDE_DIR})
    get_lib_file_name(GMP_LIB_NAME "${lib_name}")
    set_target_properties(
        ${target_name} 
        PROPERTIES IMPORTED_LOCATION 
        ${GMP_LIB_DIR}/${GMP_LIB_NAME}
    )
endfunction(add_global_imported_target)
add_global_imported_target(GMP::GMP gmp)
if(NOT GMP_ONLY)
    add_global_imported_target(GMP::GMPXX gmpxx)
endif()

