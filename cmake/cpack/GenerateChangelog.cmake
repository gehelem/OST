# cmake/cpack/GenerateChangelog.cmake
# Generates a Debian-format changelog from git history.
# Called as a cmake script (-P) at build time.
#
# Parameters (passed via -D):
#   GIT_EXECUTABLE   - path to git
#   SOURCE_DIR       - repository root
#   PACKAGE_NAME     - Debian package name
#   MAINTAINER_NAME  - maintainer display name
#   MAINTAINER_EMAIL - maintainer email
#   OUTPUT_FILE      - path to write the changelog

cmake_minimum_required(VERSION 3.10)

# --- Collect all version tags sorted by version number ---
execute_process(
    COMMAND ${GIT_EXECUTABLE} tag --list --sort=-version:refname
    WORKING_DIRECTORY "${SOURCE_DIR}"
    OUTPUT_VARIABLE _ALL_TAGS
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Filter to version-like tags only (numeric: 0.0.2, 1.0.0, etc.)
string(REPLACE "\n" ";" _TAG_LIST "${_ALL_TAGS}")
set(_VERSION_TAGS "")
foreach(_tag ${_TAG_LIST})
    if(_tag MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
        list(APPEND _VERSION_TAGS "${_tag}")
    endif()
endforeach()

# Get current HEAD sha for the unreleased block
execute_process(
    COMMAND ${GIT_EXECUTABLE} log -1 --format=%H
    WORKING_DIRECTORY "${SOURCE_DIR}"
    OUTPUT_VARIABLE _HEAD_SHA
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# --- Helper: get RFC-2822 date for a ref ---
# date format expected by Debian: "Thu, 01 Jan 2025 12:00:00 +0000"
macro(git_date _ref _out_var)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} log -1 --format=%aD "${_ref}"
        WORKING_DIRECTORY "${SOURCE_DIR}"
        OUTPUT_VARIABLE ${_out_var}
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endmacro()

# --- Helper: get commits between two refs ---
macro(git_commits_between _from _to _out_var)
    if("${_from}" STREQUAL "")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} log "${_to}" --format=%s --no-merges
            WORKING_DIRECTORY "${SOURCE_DIR}"
            OUTPUT_VARIABLE ${_out_var}
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    else()
        execute_process(
            COMMAND ${GIT_EXECUTABLE} log "${_from}..${_to}" --format=%s --no-merges
            WORKING_DIRECTORY "${SOURCE_DIR}"
            OUTPUT_VARIABLE ${_out_var}
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif()
endmacro()

# --- Build changelog content ---
set(_CHANGELOG "")

# Determine the range for HEAD (unreleased commits since last tag)
list(LENGTH _VERSION_TAGS _NTAGS)
if(_NTAGS GREATER 0)
    list(GET _VERSION_TAGS 0 _LATEST_TAG)
else()
    set(_LATEST_TAG "")
endif()

# Commits since latest tag (unreleased / snapshot block)
git_commits_between("${_LATEST_TAG}" "HEAD" _HEAD_COMMITS)
git_date("HEAD" _HEAD_DATE)

if(_HEAD_COMMITS)
    # Compute snapshot version
    if(_LATEST_TAG STREQUAL "")
        set(_SNAP_VER "0.0.0+git.snapshot")
    else()
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-list "${_LATEST_TAG}..HEAD" --count
            WORKING_DIRECTORY "${SOURCE_DIR}"
            OUTPUT_VARIABLE _COMMIT_COUNT
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        string(SUBSTRING "${_HEAD_SHA}" 0 8 _SHORT_SHA)
        set(_SNAP_VER "${_LATEST_TAG}+git${_COMMIT_COUNT}.g${_SHORT_SHA}")
    endif()

    string(APPEND _CHANGELOG "${PACKAGE_NAME} (${_SNAP_VER}-1) UNRELEASED; urgency=low\n\n")
    string(REPLACE "\n" ";" _COMMIT_LIST "${_HEAD_COMMITS}")
    foreach(_line ${_COMMIT_LIST})
        if(NOT "${_line}" STREQUAL "")
            string(APPEND _CHANGELOG "  * ${_line}\n")
        endif()
    endforeach()
    string(APPEND _CHANGELOG "\n -- ${MAINTAINER_NAME} <${MAINTAINER_EMAIL}>  ${_HEAD_DATE}\n\n")
endif()

# One entry per version tag
list(LENGTH _VERSION_TAGS _NTAGS)
math(EXPR _LAST_IDX "${_NTAGS} - 1")

foreach(_i RANGE 0 ${_LAST_IDX})
    list(GET _VERSION_TAGS ${_i} _TAG)

    # Previous tag (for range)
    math(EXPR _NEXT_IDX "${_i} + 1")
    if(_NEXT_IDX LESS _NTAGS)
        list(GET _VERSION_TAGS ${_NEXT_IDX} _PREV_TAG)
    else()
        set(_PREV_TAG "")
    endif()

    git_commits_between("${_PREV_TAG}" "${_TAG}" _TAG_COMMITS)
    git_date("${_TAG}" _TAG_DATE)

    string(APPEND _CHANGELOG "${PACKAGE_NAME} (${_TAG}-1) stable; urgency=low\n\n")

    if(_TAG_COMMITS)
        string(REPLACE "\n" ";" _COMMIT_LIST "${_TAG_COMMITS}")
        foreach(_line ${_COMMIT_LIST})
            if(NOT "${_line}" STREQUAL "")
                string(APPEND _CHANGELOG "  * ${_line}\n")
            endif()
        endforeach()
    else()
        string(APPEND _CHANGELOG "  * Release ${_TAG}\n")
    endif()

    string(APPEND _CHANGELOG "\n -- ${MAINTAINER_NAME} <${MAINTAINER_EMAIL}>  ${_TAG_DATE}\n\n")
endforeach()

# Fallback if no history at all
if(_CHANGELOG STREQUAL "")
    string(APPEND _CHANGELOG "${PACKAGE_NAME} (0.0.0-1) UNRELEASED; urgency=low\n\n")
    string(APPEND _CHANGELOG "  * Initial release\n")
    string(APPEND _CHANGELOG "\n -- ${MAINTAINER_NAME} <${MAINTAINER_EMAIL}>  ${_HEAD_DATE}\n\n")
endif()

file(WRITE "${OUTPUT_FILE}" "${_CHANGELOG}")
message(STATUS "Changelog written to ${OUTPUT_FILE}")
