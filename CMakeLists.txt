cmake_minimum_required(VERSION 3.10)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ParseVersion.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/BuildType.cmake)

# Configure project
project(librtp VERSION ${LIBRTP_VERSION} LANGUAGES C CXX)
message(STATUS "RTP library version: ${LIBRTP_VERSION} [${CMAKE_BUILD_TYPE}]")

set(PROJECT_DESCRIPTION "Real-time Transport Protocol")
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

include(GNUInstallDirs)

# Options
option(LIBRTP_BUILD_TESTS "Build testing" OFF)
option(LIBRTP_BUILD_DOCS "Build documentation" OFF)
option(LIBRTP_BUILD_EXAMPLES "Build examples" OFF)

# Build library
add_subdirectory(include)
add_subdirectory(source)

add_library(rtp SHARED ${RTP_SOURCES})

set_target_properties(rtp PROPERTIES
    SOVERSION ${LIBRTP_VERSION_MAJOR}
    VERSION ${LIBRTP_VERSION}
    PUBLIC_HEADER "${RTP_HEADERS}")

target_include_directories(rtp PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
    $<INSTALL_INTERFACE:include/>)

if(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(rtp PRIVATE
        -Wall -Wextra -Wpedantic -Wmissing-prototypes)
endif()

install(
    TARGETS rtp EXPORT RtpTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rtp)

# Build tests (-DLIBRTP_BUILD_TESTS=ON)
if(LIBRTP_BUILD_TESTS)
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

# Install CMake configuration export file.
include(CMakePackageConfigHelpers)

set(LIBRTP_CMAKE_DESTINATION share/rtp)
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR})

configure_package_config_file(
    cmake/RtpConfig.cmake.in RtpConfig.cmake
    INSTALL_DESTINATION ${LIBRTP_CMAKE_DESTINATION}
    PATH_VARS INCLUDE_INSTALL_DIR
    INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/RtpConfigVersion.cmake
    VERSION ${LIBRTP_VERSION}
    COMPATIBILITY SameMajorVersion)

install(
    FILES
        ${CMAKE_BINARY_DIR}/RtpConfig.cmake
        ${CMAKE_BINARY_DIR}/RtpConfigVersion.cmake
    DESTINATION
        ${LIBRTP_CMAKE_DESTINATION})

install(EXPORT RtpTargets
        NAMESPACE rtp::
        DESTINATION ${LIBRTP_CMAKE_DESTINATION})

# Prepare CPack
set(CPACK_PACKAGE_NAME librtp)
set(CPACK_PACKAGE_DESCRIPTION ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "Nova Dynamics LLC")
set(CPACK_PACKAGE_CONTACT "ww@novadynamics.com")

set(CPACK_PACKAGE_VERSION_MAJOR "${LIBRTP_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${LIBRTP_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${LIBRTP_VERSION_PATCH}")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

if(WIN32)
    # cpack -G NSIS
    set(CPACK_GENERATOR NSIS)
	set(CPACK_PACKAGE_INSTALL_DIRECTORY "rtp")
    set(CPACK_NSIS_DISPLAY_NAME "rtp")
    set(CPACK_NSIS_PACKAGE_NAME "rtp")
else()
    # cpack -G DEB
    set(CPACK_GENERATOR DEB)
	set(CPACK_PACKAGE_INSTALL_DIRECTORY "rtp")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Wilkins White")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
endif()

include(CPack)
