cmake_minimum_required(VERSION 3.13)

#If cache variable is not set, use same library type as project.
if(NOT ANDROID AND "${OpenSSL_LIBRARY_TYPE}" STREQUAL "")
    set(OpenSSL_LIBRARY_TYPE "${${PROJECT_NAME}_LIBRARY_TYPE}")
#Emit an info message if library was set to another type than static for android builds. 
elseif(ANDROID AND NOT "${OpenSSL_LIBRARY_TYPE}" STREQUAL "" AND NOT "${OpenSSL_LIBRARY_TYPE}" STREQUAL "STATIC")
    set(OpenSSL_LIBRARY_TYPE "STATIC")
    message(STATUS "Only static OpenSSL builds supported for Android.")
#Set OpenSSL to static without emiting a status message, if cache variable is not set.
elseif(ANDROID)
    set(OpenSSL_LIBRARY_TYPE "STATIC")
endif()

#Try to find OpenSSL, unless user requests that OpenSSL must be built.
if(NOT FORCE_SSL_COMPILATION)
    find_package(OpenSSL QUIET)
    #Retry in Config mode, if module mode didn't find OpenSSL
    if(NOT OPENSSL_FOUND)
        find_package(OpenSSL QUIET CONFIG)
    endif()
    if(TARGET OpenSSL::SSL AND TARGET OpenSSL::Crypto)
        set(OPENSSL_FOUND TRUE)
    endif()
    if(OPENSSL_FOUND)
        message(STATUS "Found OpenSSL.")
    endif()
else()
    set(OPENSSL_FOUND FALSE)
endif()

if(NOT OPENSSL_FOUND)
    set(PFX ${CMAKE_${OpenSSL_LIBRARY_TYPE}_LIBRARY_PREFIX})
    set(SFX ${CMAKE_${OpenSSL_LIBRARY_TYPE}_LIBRARY_SUFFIX})
    set(SSL_LIBRARY_NAME ${PFX}ssl${SFX})
    set(CRYPTO_LIBRARY_NAME ${PFX}crypto${SFX})
    include(ExternalBuildHelper)
    #openssl library can be found at user provided locations.
    if(EXISTS "${OpenSSL_LIBRARY_DIR}/${SSL_LIBRARY_NAME}"
       AND EXISTS "${OpenSSL_LIBRARY_DIR}/${CRYPTO_LIBRARY_NAME}" 
       AND EXISTS "${OpenSSL_INCLUDES}")
        message(STATUS "Found OpenSSL at given location.")
        add_imported_library(TARGET OpenSSL::SSL ${OpenSSL_LIBRARY_TYPE}
                             EXTERNAL_LIB_DIR "${OpenSSL_LIBRARY_DIR}"
                             EXTERNAL_LIB_NAME ssl
                             EXTERNAL_INCLUDES "${OpenSSL_INCLUDES}")
        add_imported_library(TARGET OpenSSL::Crypto ${OpenSSL_LIBRARY_TYPE}
                             EXTERNAL_LIB_DIR "${OpenSSL_LIBRARY_DIR}"
                             EXTERNAL_LIB_NAME crypto
                             EXTERNAL_INCLUDES "${OpenSSL_INCLUDES}")
    #If openssl library cannot be found, but is allowed to be build.
    elseif(BUILD_OpenSSL OR FORCE_OpenSSL_BUILD)
        message(STATUS "Adding OpenSSL library to build.")
        include(BuildOpenSSL)
    #Emit an error message if openssl library or include directories cannot be found in cached variables
    #and either option BUILD_OpenSSL or FORCE_OpenSSL_BUILD is not set.
    else()
        message(SEND_ERROR "Did not find openssl in standard location." 
                           " Either set OpenSSL_LIBRARY_DIR and OpenSSL_INCLUDES to valid locations" 
                           " or enable OpenSSL build by setting BUILD_OpenSSL. ")
        return()
    endif()
    #Install openssl libraries.
    install_imported_library("OpenSSL::SSL;OpenSSL::Crypto" "OpenSSL")
endif()
