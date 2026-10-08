# cmake/Frontend.cmake
#
# Builds front/'s static www/ output at CMake build time - issue #229's
# first packaging step, for the future "complete" CPack component/Debian
# package to install() from (not wired into that yet, this file only
# produces the artifact).
#
# Deliberately NOT a Node/npm step: front/package.json's own "prepare-www"
# script does the equivalent (same file list, same gitdata.json via
# front/CI/write-gitdata.js) for its own consumers (local dev server,
# Capacitor Android build, both already need Node anyway) - this is a
# parallel, dependency-free reimplementation for the CMake-driven
# packaging path specifically, so building the "complete" package never
# adds `node` to this backend's own Build-Depends (this project has zero
# JS tooling dependency today, intentionally - see front/'s own "no
# bundler" convention, same spirit applied here one level up).
#
# `frontend_www` is deliberately NOT a dependency of the `ostserver`
# target (see CMakeLists.txt) - the minimal backend-only package never
# needs front/ at all, only the future "complete" package does. It's
# still built under the default ALL target for convenience during a full
# local build.

set(FRONTEND_SOURCE_DIR "${CMAKE_SOURCE_DIR}/front")
set(FRONTEND_WWW_DIR "${CMAKE_BINARY_DIR}/front-www")

# Same file list as front/package.json's own "prepare-www" script -
# index.html plus these directories, verbatim copy, no transformation
# (front/ ships plain ES modules, no bundler, nothing to compile here).
set(FRONTEND_WWW_SUBDIRS js css assets core lib celestial-data)

# NOTE: file(GLOB_RECURSE ...) is evaluated at CMake *configure* time, not
# re-evaluated automatically when a file is added/removed under front/ -
# a known CMake limitation (editing an existing file's content still
# retriggers this custom command correctly; adding a brand new file needs
# a fresh `cmake` reconfigure to be picked up as a new dependency). Same
# tradeoff already accepted elsewhere in this project's own CMake set-up;
# not worth a more complex always-re-copy scheme for a handful of mostly-
# stable static assets.
file(GLOB_RECURSE FRONTEND_SOURCE_FILES
    "${FRONTEND_SOURCE_DIR}/index.html"
    "${FRONTEND_SOURCE_DIR}/js/*"
    "${FRONTEND_SOURCE_DIR}/css/*"
    "${FRONTEND_SOURCE_DIR}/assets/*"
    "${FRONTEND_SOURCE_DIR}/core/*"
    "${FRONTEND_SOURCE_DIR}/lib/*"
    "${FRONTEND_SOURCE_DIR}/celestial-data/*"
)

set(_frontend_www_stamp "${CMAKE_BINARY_DIR}/front-www.stamp")

set(_copy_commands
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${FRONTEND_WWW_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${FRONTEND_WWW_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy "${FRONTEND_SOURCE_DIR}/index.html" "${FRONTEND_WWW_DIR}/index.html"
)
foreach(_subdir ${FRONTEND_WWW_SUBDIRS})
    list(APPEND _copy_commands
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${FRONTEND_SOURCE_DIR}/${_subdir}" "${FRONTEND_WWW_DIR}/${_subdir}"
    )
endforeach()

add_custom_command(
    OUTPUT "${_frontend_www_stamp}"
    ${_copy_commands}
    COMMAND ${CMAKE_COMMAND}
        -DGIT_EXECUTABLE=${GIT_EXECUTABLE}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DOUTPUT_FILE=${FRONTEND_WWW_DIR}/gitdata.json
        -P "${CMAKE_CURRENT_LIST_DIR}/WriteGitData.cmake"
    COMMAND ${CMAKE_COMMAND} -E touch "${_frontend_www_stamp}"
    DEPENDS ${FRONTEND_SOURCE_FILES}
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Preparing frontend www/ output from front/ (no Node/npm involved)"
    VERBATIM
)

add_custom_target(frontend_www ALL DEPENDS "${_frontend_www_stamp}")
