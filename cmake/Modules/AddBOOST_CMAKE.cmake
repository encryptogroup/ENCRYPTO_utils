cmake_minimum_required(VERSION 3.13)

set(BOOST_CMAKE_SOURCE "${BOOST_CMAKE_SOURCE}" CACHE PATH "Path to boost-cmake source.")
set(BOOST_CMAKE_URL https://github.com/Orphis/boost-cmake/archive/f0f64adda5ac1312549056dab502da55b9c45b7b.zip CACHE STRING "URL to boost-cmake project.")
set(BOOST_CMAKE_URL_HASH SHA256=0be1ccdebe0a4f77dae683273984770937dd5ef4ee1f5a7552bf862e478f81dd CACHE STRING "Hash of boost-cmake archive.")

set(BOOST_INSTALL_INCLUDE "${${PROJECT_NAME}_INSTALL_INCLUDE}")
set(USE_ANDROID ANDROID)
file(GLOB BOOST_CMAKE_FILE_LIST "${PROJECT_SOURCE_DIR}/extern/boost-cmake/*")
list(LENGTH BOOST_CMAKE_FILE_LIST BOOST_CMAKE_NUM_FILES)
#if boost-cmake directory is empty
if(BOOST_CMAKE_NUM_FILES EQUAL 0)
    include(FetchBOOST_CMAKE)
else()
    set(BOOST_CMAKE_SOURCE "${PROJECT_SOURCE_DIR}/extern/BOOST_CMAKE"
        CACHE PATH 
        "Path to boost-cmake source."
        FORCE
    )
    include(FetchBOOST_CMAKE)
endif()
