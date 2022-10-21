file(
    STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/include/version.h"
    VERSION_PARTS REGEX "#define LIBRTP_VERSION_[A-Z]+[ ]+")

string(
    REGEX REPLACE ".+LIBRTP_VERSION_MAJOR[ ]+([0-9]+).*" "\\1"
    LIBRTP_VERSION_MAJOR "${VERSION_PARTS}")

string(
    REGEX REPLACE ".+LIBRTP_VERSION_MINOR[ ]+([0-9]+).*" "\\1"
    LIBRTP_VERSION_MINOR "${VERSION_PARTS}")

string(
    REGEX REPLACE ".+LIBRTP_VERSION_PATCH[ ]+([0-9]+).*" "\\1"
    LIBRTP_VERSION_PATCH "${VERSION_PARTS}")

set(LIBRTP_VERSION "${LIBRTP_VERSION_MAJOR}.${LIBRTP_VERSION_MINOR}.${LIBRTP_VERSION_PATCH}")
