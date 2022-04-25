cmake_minimum_required(VERSION 3.10)

# Configure project
include(cmake/ParseVersion.cmake)
project(LIBRTP VERSION ${LIBRTP_VERSION} LANGUAGES C CXX)
message(STATUS "RTP library version: ${LIBRTP_VERSION}")

set(PROJECT_DESCRIPTION "Real-time Transport Protocol")
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

include(GNUInstallDirs)

# Options
option(LIBRTP_BUILD_TESTING "Build testing" OFF)
option(LIBRTP_BUILD_DOCS "Build documentation" OFF)
option(LIBRTP_BUILD_EXAMPLES "Build examples" OFF)

# Build library
add_subdirectory(include)
add_subdirectory(source)

add_library(rtp ${RTP_SOURCES})

set_target_properties(rtp PROPERTIES
    SOVERSION ${LIBRTP_VERSION_MAJOR}
    VERSION ${LIBRTP_VERSION}
    PUBLIC_HEADER "${RTP_HEADERS}")

target_include_directories(rtp PUBLIC ${PROJECT_SOURCE_DIR}/include)

if(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(rtp PRIVATE
        -Wall -Wextra -Wpedantic -Wmissing-prototypes)
endif()

install(
    TARGETS rtp
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rtp)

# Build tests (-DLIBRTP_BUILD_TESTING=ON)
if(LIBRTP_BUILD_TESTING AND BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()

# Build examples (-DLIBRTP_BUILD_EXAMPLES=ON)
if(LIBRTP_BUILD_EXAMPLES)
    find_package(PulseAudio)
    if(PulseAudio_FOUND)
        add_subdirectory(examples)
    else()
        message(STATUS "PulseAudio not found - skipping examples")
    endif()
endif()

# Build documentation (-DLIBRTP_BUILD_DOCS=ON)
if(LIBRTP_BUILD_DOCS)
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
        message(STATUS "Doxygen not found - skipping docs")
    endif()
endif()
