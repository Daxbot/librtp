cmake_minimum_required(VERSION 3.10)

# Configure project
include(cmake/ParseVersion.cmake)
project(LIBRTP VERSION ${LIBRTP_VERSION} LANGUAGES C CXX)

set(PROJECT_DESCRIPTION "Real-time Transport Protocol")
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

include(GNUInstallDirs)

# Options
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_DOCS "Build documentation" OFF)
option(BUILD_EXAMPLES "Build PulseAudio examples" OFF)


# Build tests (-DBUILD_TESTS=ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

# Build examples (-DBUILD_EXAMPLES=ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Build documentation (-DBUILD_DOCS=ON)
if(BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        configure_file(
            ${CMAKE_SOURCE_DIR}/cmake/Doxyfile.in
            ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            @ONLY)

        add_custom_target(docs ALL
            COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating documentation"
            VERBATIM)
    else()
        message("Doxygen is not installed")
    endif()
endif()

# Build library
add_subdirectory(include)
add_subdirectory(source)

add_library(rtp SHARED ${RTP_SOURCES})

set_target_properties(rtp PROPERTIES PUBLIC_HEADER "${RTP_HEADERS}")

target_include_directories(rtp PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_compile_options(rtp PRIVATE -Wall -Wextra)

install(
    TARGETS rtp
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rtp)
