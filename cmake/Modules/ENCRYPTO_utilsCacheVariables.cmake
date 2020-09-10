set(ENCRYPTO_utils_LIBRARY_TYPE "${ENCRYPTO_utils_LIBRARY_TYPE}" CACHE STRING
    [=["[STATIC | SHARED | MODULE] The type of library in which ENCRYPTO_utils will be built. "
       "Default: STATIC"]=]
)
set_property(CACHE ENCRYPTO_utils_LIBRARY_TYPE PROPERTY STRINGS STATIC SHARED MODULE)
string(TOUPPER "${ENCRYPTO_utils_LIBRARY_TYPE}" ENCRYPTO_utils_LIBRARY_TYPE)
if("${ENCRYPTO_utils_LIBRARY_TYPE}" STREQUAL "")
    set(ENCRYPTO_utils_LIBRARY_TYPE "STATIC")
elseif(NOT "${ENCRYPTO_utils_LIBRARY_TYPE}" STREQUAL "STATIC" AND
       NOT "${ENCRYPTO_utils_LIBRARY_TYPE}" STREQUAL "SHARED" AND
       NOT "${ENCRYPTO_utils_LIBRARY_TYPE}" STREQUAL "MODULE")
    message(WARNING 
        "Unknown library type: ${ENCRYPTO_utils_LIBRARY_TYPE}. "
        "Setting ENCRYPTO_utils_LIBRARY_TYPE to default value."
    )
    set(ENCRYPTO_utils_LIBRARY_TYPE "SHARED")
endif()

set(DEPENDENCY_DIR "${DEPENDENCY_DIR}" CACHE PATH
    "Path to directory, where dependencies will be downloaded."
)

# Set build type to `Release` if none was specified:
# (cf. https://gitlab.kitware.com/cmake/community/wikis/FAQ#how-can-i-change-the-default-build-mode-and-see-it-reflected-in-the-gui)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release 
        CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
                 "None" "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif(NOT CMAKE_BUILD_TYPE)

include(AndroidCacheVariables)

#Cache Variables related to GMP dependency
option(BUILD_GMP "Build GMP library if none is found." OFF)
option(FORCE_GMP_BUILD "Force building of GMP library (use if installed GMP library is not compatible with build)." OFF)
set(GMP_LIBRARY_DIR
    CACHE PATH "Path to GMP library.")
set(GMP_INCLUDES 
    CACHE PATH "Path to GMP include directories.")
set(GMP_SOURCE
    CACHE PATH "Path to GMP source (If building GMP).")
set(GMP_URL https://gmplib.org/download/gmp/gmp-6.2.0.tar.lz
    CACHE STRING "URL of GMP source.")
set(GMP_URL_HASH SHA512=9975e8766e62a1d48c0b6d7bbdd2fccb5b22243819102ca6c8d91f0edd2d3a1cef21c526d647c2159bb29dd2a7dcbd0d621391b2e4b48662cf63a8e6749561cd 
    CACHE STRING "Hash of GMP source archive.")
set(GMP_LIBRARY_TYPE "${GMP_LIBRARY_TYPE}"
    CACHE STRING "[SHARED | STATIC]: Type of GMP library linked to project.")
set_property(CACHE GMP_LIBRARY_TYPE PROPERTY STRINGS STATIC SHARED)
mark_as_advanced(FORCE_GMP_BUILD)

#Cache Variables related to OpenSSL dependency
option(BUILD_OpenSSL "Build OpenSSL library if none is found." OFF)
option(FORCE_OpenSSL_BUILD "Force building of OpenSSL library (use if installed OpenSSL library is incompatible with build)." OFF)
mark_as_advanced(FORCE_OpenSSL_BUILD)
set(OpenSSL_LIBRARY_DIR
    CACHE PATH "Path to OpenSSL library.")
set(OpenSSL_INCLUDES
    CACHE PATH "Path to OpenSSL include directories.")
set(OpenSSL_SOURCE
    CACHE PATH "Path to OpenSSL source, when building OpenSSL.")
#We use a URL to the OpenSSL repository, as it is the simplest way to avoid the following problem:
#When using a git repository OpenSSL and all dependencies gets rebuilt after make install, even when
#make was called previously. Furthermore, using a URL to a zip archive drastically reduces the download time.
set(OpenSSL_URL https://github.com/openssl/openssl/archive/OpenSSL_1_1_1.zip
    CACHE STRING "URL to OpenSSL source of OpenSSL_1_1_1 release.")
set(OpenSSL_URL_HASH SHA512=9db4bf391739f4e835bcfda10533d49a7231508faf25d88d880dfadc98ffd3b503d58879368ff17bed33d7775c1f0eb6991aa71d6888f50c5ad65b2d221be18e
    CACHE STRING "Hash of OpenSSL source archive.")
set(OpenSSL_VERSION 1.1 
    CACHE STRING "Shared library version of OpenSSL, e.g. libssl.so.1.1, with OpenSSL_VERSION=1.1")
set(OpenSSL_LIBRARY_TYPE
    CACHE STRING "[SHARED | STATIC]: Type of OpenSSL library linked to project.")
set_property(CACHE OpenSSL_LIBRARY_TYPE PROPERTY STRINGS STATIC SHARED)

#Cache Variables related to Relic dependency
set(Relic_SOURCE
    CACHE PATH "Path to Relic source.")
set(Relic_REPOSITORY https://github.com/relic-toolkit/relic.git
    CACHE STRING "Git repository of Relic project.")
set(Relic_TAG refs/tags/relic-toolkit-0.5.0 
    CACHE STRING "Git tag of downloaded Relic project.")
set(Relic_LIBRARY_TYPE CACHE STRING "[SHARED | STATIC]: Type of Relic library linked to project.")
set_property(CACHE Relic_LIBRARY_TYPE PROPERTY STRINGS STATIC SHARED)
if(ANDROID)
    if(NOT "${Relic_LIBRARY_TYPE}" STREQUAL "" AND NOT "${Relic_LIBRARY_TYPE}" STREQUAL "STATIC")
        message(WARNING "${Relic_LIBRARY_TYPE} build for Relic is disabled on Android, " 
                        "setting Relic_LIBRARY_TYPE to STATIC...")
    endif()
    set(Relic_LIBRARY_TYPE "STATIC")
else()
    if("${Relic_LIBRARY_TYPE}" STREQUAL "")
        set(Relic_LIBRARY_TYPE "${ENCRYPTO_utils_LIBRARY_TYPE}")
    endif()
endif()
set(LABEL "aby" CACHE STRING "Label for relic (empty label not recommended, as this might cause name conflicts at link time)")
set(DEBUG off CACHE BOOL "Build relic with debugging support")
set(PROFL off CACHE BOOL "Build relic with profiling support")
set(CHECK off CACHE BOOL "Build relic with error-checking support")
set(ALIGN "16" CACHE STRING "Relic align")
set(ARITH "curve2251-sse" CACHE STRING "arithmetic utils used in relic")
set(FB_POLYN ${ecclvl} CACHE STRING "security level of the ecc binary curve in relic")
set(FB_METHD "INTEG;INTEG;QUICK;QUICK;QUICK;QUICK;QUICK;SLIDE;QUICK" CACHE STRING "Methods for fb in relic")
set(FB_PRECO on CACHE BOOL "fb preco for relic")
set(FB_SQRTF off CACHE BOOL "sqrtf for relic")
set(EB_METHD "PROJC;LODAH;COMBS;INTER" CACHE STRING "Methods for eb in relic")
set(EC_METHD "CHAR2" CACHE STRING "Methods for ec in relic")
set(TIMER "CYCLE" CACHE STRING "Relic timer")
set(TESTS "0" CACHE STRING "Relic amount of random tests, 0 for disable")
set(BENCH "0" CACHE STRING "Relic amount of benchmarks on random values, 0 for disable")
set(WITH "MD;DV;BN;FB;EB;EC" CACHE STRING "Relic algorithms")
set(COMP "-O3 -funroll-loops -fomit-frame-pointer -march=core2 -msse4.2 -mpclmul" CACHE STRING "Relic compiler options")
set(ARCH "X64" CACHE STRING "Architecture to be used in relic")
set(WSIZE "64" CACHE STRING "Relic word size in bits")
if("${Relic_LIBRARY_TYPE}" STREQUAL "STATIC")
    set(SHLIB off CACHE BOOL "Relic shared library")
    set(STLIB on CACHE BOOL "Relic static library")
elseif("${Relic_LIBRARY_TYPE}" STREQUAL "SHARED")
    set(SHLIB on CACHE BOOL "Relic shared library")
    set(STLIB off CACHE BOOL "Relic static library")
endif()
#Overwrite cache entries to be consistent with target android platform
if(ANDROID AND (ANDROID_ABI STREQUAL "armeabi-v7a" OR ANDROID_ABI STREQUAL "x86" OR ANDROID_ABI STREQUAL "mips"))
    set(WSIZE "32")
else()
    set(WSIZE "64")
endif()
if(ANDROID)
    set(COMP "-O3 -funroll-loops -fomit-frame-pointer")
    set(OPSYS "DROID")
    if(ANDROID_ABI STREQUAL "armeabi-v7a")
        set(ARITH "arm-asm-254")
        set(ARCH "ARM")
    elseif(ANDROID_ABI STREQUAL "arm64-v8a")
        set(ARITH "arm-asm-254")
        set(ARCH "")
    elseif(ANDROID_ABI STREQUAL "x86")
        set(ARITH "fiat")
        set(ARCH "X86")
    elseif(ANDROID_ABI STREQUAL "x86_64")
        set(ARITH "fiat")
        set(ARCH "X64")
    endif()
endif()
