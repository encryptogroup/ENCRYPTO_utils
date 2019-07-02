add_library(encrypto_utils
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
    ${PROJECT_NAME}/parse_options.cpp
    ${PROJECT_NAME}/powmod.cpp
    ${PROJECT_NAME}/rcvthread.cpp
    ${PROJECT_NAME}/sndthread.cpp
    ${PROJECT_NAME}/socket.cpp
    ${PROJECT_NAME}/thread.cpp
    ${PROJECT_NAME}/timer.cpp
    ${PROJECT_NAME}/utils.cpp
)
add_library(ENCRYPTO_utils::encrypto_utils ALIAS encrypto_utils)

target_compile_features(encrypto_utils PUBLIC cxx_std_17)
target_compile_options(encrypto_utils PRIVATE "-Wall" "-Wextra")
#Maybe it is better to create a ecc-pk-crypti.h.in file, since the ecclvl is only needed there
target_compile_definitions(encrypto_utils PUBLIC ECCLVL=${ecclvl})

target_include_directories(encrypto_utils
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)


target_link_libraries(encrypto_utils
    PRIVATE
        Boost::system
        Boost::thread
    PUBLIC
        GMP::GMP
        GMP::GMPXX
        OpenSSL::Crypto
        RELIC::relic
)

install(TARGETS encrypto_utils
        EXPORT "${PROJECT_NAME}Targets"
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        INCLUDES DESTINATION include
)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
        DESTINATION include
        FILES_MATCHING PATTERN "*.h"
)
export(TARGETS encrypto_utils NAMESPACE "${PROJECT_NAME}::" FILE "${PROJECT_NAME}Targets.cmake")
install(EXPORT "${PROJECT_NAME}Targets"
        NAMESPACE "${PROJECT_NAME}::"
        DESTINATION "lib/cmake/${PROJECT_NAME}"
)


include(CMakePackageConfigHelpers)

configure_package_config_file("${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
	${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION "lib/cmake/${PROJECT_NAME}"
)

install(FILES
	"${PROJECT_SOURCE_DIR}/cmake/FindGMP.cmake"
    "${PROJECT_SOURCE_DIR}/cmake/FindGMPXX.cmake"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    DESTINATION "lib/cmake/${PROJECT_NAME}"
)

