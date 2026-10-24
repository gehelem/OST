# cmake/WriteGitData.cmake
#
# Standalone script (run via `cmake -P`, not part of the normal configure
# pass) that writes front-www/gitdata.json - same shape front/CI/
# write-gitdata.js already writes for the npm/Capacitor build path
# ({tag, commit, date, message}), reimplemented here purely in CMake so
# the backend-driven "complete" package build never needs `node` as a
# Build-Depends (see cmake/Frontend.cmake's own header comment - this
# backend has zero JS tooling dependency today, intentionally kept that
# way).
#
# Run from the now-unified repo root (issue #229: front/ lives inside
# this repo) - tag/commit/date/message describe the WHOLE repo's latest
# commit, not just front/'s own history. That's the intended behavior
# post-merge (one version identifies the whole system, backend+frontend
# together), not an oversight - see issue #229's own packaging rationale.
#
# Expected variables (passed via -D on the `cmake -P` command line):
#   GIT_EXECUTABLE - path to git (optional, auto-detected if omitted)
#   SOURCE_DIR     - repo root to run git commands from
#   OUTPUT_FILE    - where to write the resulting gitdata.json

if(NOT GIT_EXECUTABLE)
    find_program(GIT_EXECUTABLE git)
endif()

function(git_or_fallback out_var)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} ${ARGN}
        WORKING_DIRECTORY "${SOURCE_DIR}"
        OUTPUT_VARIABLE _result
        RESULT_VARIABLE _rc
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT _rc EQUAL 0 OR _result STREQUAL "")
        set(_result "unknown")
    endif()
    set(${out_var} "${_result}" PARENT_SCOPE)
endfunction()

execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --git-dir
    WORKING_DIRECTORY "${SOURCE_DIR}"
    RESULT_VARIABLE _in_git_repo
    OUTPUT_QUIET ERROR_QUIET
)

if(_in_git_repo EQUAL 0)
    # Same four fields, same git subcommands, same fallback-to-"unknown" on
    # failure as front/CI/write-gitdata.js's own git() helper - "tag" falls
    # back to the short commit hash when no tag is reachable (any commit
    # between two releases) rather than failing outright.
    git_or_fallback(_tag describe --tags --always)
    git_or_fallback(_commit rev-parse --short HEAD)
    git_or_fallback(_date log -1 --format=%cI)
    git_or_fallback(_message log -1 --format=%s)
else()
    # No .git here at all - a Launchpad build, which builds from a
    # tarball export of the tag/branch tip with no git history. Same
    # debian/changelog fallback as generate-version.sh's own backend
    # equivalent, so the front half of the credits dialog isn't stuck on
    # a hardcoded "unknown" every single field whenever this path is hit.
    find_program(DPKG_PARSECHANGELOG_EXECUTABLE dpkg-parsechangelog)
    function(dpkg_field_or_fallback out_var field)
        execute_process(
            COMMAND ${DPKG_PARSECHANGELOG_EXECUTABLE} --show-field ${field}
            WORKING_DIRECTORY "${SOURCE_DIR}"
            OUTPUT_VARIABLE _result
            RESULT_VARIABLE _rc
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(NOT _rc EQUAL 0 OR _result STREQUAL "")
            set(_result "unknown")
        endif()
        set(${out_var} "${_result}" PARENT_SCOPE)
    endfunction()
    dpkg_field_or_fallback(_tag Version)
    dpkg_field_or_fallback(_date Date)
    set(_commit "unknown")
    set(_message "Launchpad build")
endif()

# Minimal, dependency-free JSON string escaping - a one-line commit
# subject realistically only risks a literal backslash or double-quote,
# no control characters expected.
string(REPLACE "\\" "\\\\" _message "${_message}")
string(REPLACE "\"" "\\\"" _message "${_message}")

file(WRITE "${OUTPUT_FILE}"
"{
  \"tag\": \"${_tag}\",
  \"commit\": \"${_commit}\",
  \"date\": \"${_date}\",
  \"message\": \"${_message}\"
}
")

message(STATUS "wrote ${OUTPUT_FILE}: tag=${_tag} commit=${_commit}")
