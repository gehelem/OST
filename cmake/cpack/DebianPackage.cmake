# cmake/cpack/DebianPackage.cmake
# CPack Debian packaging configuration with dynamic versioning and changelog

# ---------------------------------------------------------------------------
# Version from git
# ---------------------------------------------------------------------------
# Strategy:
#   - Tagged commit (e.g. tag "0.0.2"):          version = 0.0.2-1
#   - N commits after tag (e.g. 0.0.2, +138):    version = 0.0.2+git138.gabcdef12-1
#   - No tags at all:                             version = 0.0.0+git.YYYYMMDD.gabcdef12-1

find_package(Git QUIET)

set(OST_DEBIAN_REVISION "1")  # Increment when re-packaging same upstream version

if(Git_FOUND)
    # git describe --tags --long --match "v*" or plain numeric tags
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --long --match "[0-9]*"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE _GIT_DESCRIBE
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(_GIT_DESCRIBE MATCHES "^([0-9]+\\.[0-9]+\\.[0-9]+)-([0-9]+)-g([0-9a-f]+)$")
        set(_TAG_VERSION   "${CMAKE_MATCH_1}")
        set(_COMMITS_SINCE "${CMAKE_MATCH_2}")
        set(_GIT_SHA       "${CMAKE_MATCH_3}")

        if(_COMMITS_SINCE STREQUAL "0")
            # Exactly on a tag
            set(OST_UPSTREAM_VERSION "${_TAG_VERSION}")
        else()
            # Commits after a tag → pre-release / snapshot
            set(OST_UPSTREAM_VERSION "${_TAG_VERSION}+git${_COMMITS_SINCE}.g${_GIT_SHA}")
        endif()
    else()
        # No version tag found — use date + sha
        execute_process(
            COMMAND ${GIT_EXECUTABLE} log -1 --format=%cd --date=format:%Y%m%d
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE _GIT_DATE
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        execute_process(
            COMMAND ${GIT_EXECUTABLE} log -1 --format=%h
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE _GIT_SHA
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        set(OST_UPSTREAM_VERSION "0.0.0+git.${_GIT_DATE}.g${_GIT_SHA}")
    endif()
else()
    set(OST_UPSTREAM_VERSION "0.0.0")
endif()

set(OST_DEBIAN_VERSION "${OST_UPSTREAM_VERSION}-${OST_DEBIAN_REVISION}")

message(STATUS "Package version: ${OST_DEBIAN_VERSION}")

# ---------------------------------------------------------------------------
# Changelog generation
# ---------------------------------------------------------------------------
# Generates a Debian-compliant changelog from git log at build time.
# Each git tag becomes a changelog entry; commits between tags are listed.

set(OST_CHANGELOG_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/GenerateChangelog.cmake")
set(OST_CHANGELOG_OUT    "${CMAKE_BINARY_DIR}/changelog.Debian")
set(OST_CHANGELOG_GZ     "${CMAKE_BINARY_DIR}/changelog.Debian.gz")

add_custom_command(
    OUTPUT "${OST_CHANGELOG_GZ}"
    COMMAND ${CMAKE_COMMAND}
        -DGIT_EXECUTABLE=${GIT_EXECUTABLE}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DPACKAGE_NAME=ostserver
        -DMAINTAINER_NAME="gilles"
        -DMAINTAINER_EMAIL="gilles@joag.fr"
        -DOUTPUT_FILE=${OST_CHANGELOG_OUT}
        -P "${OST_CHANGELOG_SCRIPT}"
    COMMAND gzip -9 -n -f "${OST_CHANGELOG_OUT}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Generating Debian changelog from git log"
    VERBATIM
)
add_custom_target(debian_changelog ALL DEPENDS "${OST_CHANGELOG_GZ}")
add_dependencies(ostserver debian_changelog)

install(
    FILES "${OST_CHANGELOG_GZ}"
    DESTINATION "share/doc/ostserver"
)

# ---------------------------------------------------------------------------
# CPack / DEB configuration
# ---------------------------------------------------------------------------
set(CPACK_GENERATOR "DEB")

set(CPACK_PACKAGE_NAME        "ostserver")
set(CPACK_PACKAGE_VERSION     "${OST_UPSTREAM_VERSION}")
set(CPACK_PACKAGE_RELEASE     "${OST_DEBIAN_REVISION}")
set(CPACK_PACKAGE_CONTACT     "gilles <gilles@joag.fr>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Observatoire Sans Tête")
set(CPACK_PACKAGE_DESCRIPTION
"Observatoire Sans Tête
 Light and headless astrophotography automation tool")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/gehelem/OST")

# Output filename: ostserver_0.0.2+git138.gabcdef12-1_amd64.deb
set(CPACK_PACKAGE_FILE_NAME
    "${CPACK_PACKAGE_NAME}_${OST_DEBIAN_VERSION}_${CMAKE_SYSTEM_PROCESSOR}")

set(CPACK_DEB_COMPONENT_INSTALL OFF)

set(CPACK_DEBIAN_PACKAGE_SECTION      "science")
set(CPACK_DEBIAN_PACKAGE_PRIORITY     "optional")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER   "gilles <gilles@joag.fr>")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE     "https://github.com/gehelem/OST")

# Runtime dependencies
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "libqt5core5a, libqt5network5, libqt5websockets5, libqt5sql5, libqt5sql5-sqlite")
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS
    "libindi1, libstellarsolver")

# Let dpkg-shlibdeps auto-detect shared library dependencies
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)
