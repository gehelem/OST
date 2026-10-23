#!/bin/sh
#
# CI - Sync debian/changelog's version to the current git state
#
# Launchpad's PPA build (git-build-recipe) reads debian/changelog's top
# version as plain text, before any of our own build tooling ever runs,
# from a tarball export with no .git directory - it can't derive a
# version from git history itself the way CPack/RPM/Arch builds already
# do (see DebianPackage.cmake's OST_UPSTREAM_VERSION). So that version
# has to be computed and committed ahead of time instead, here. Same
# "<tag>" / "<tag>+git<n>.g<sha>" scheme as DebianPackage.cmake, for
# consistency across every package format.
#
# Only meaningful on an actual push landing on main (or a tag, which is
# a push too) - never on a merge-request validation pipeline, since
# main hasn't received that branch's commits yet at that point.
#
set -e

if [ "${CI_PIPELINE_SOURCE}" != "push" ]; then
    echo "not a push pipeline (source=${CI_PIPELINE_SOURCE}), skipping debian/changelog sync"
    exit 0
fi

# A tag push already tells us the exact version via CI_COMMIT_TAG - no
# need to re-derive it from git describe, which would be sensitive to
# whatever HEAD the calling script left behind (e.g. CI-upload-release-tag.sh's
# own CHANGELOG.md commit, pushed just before this runs).
if [ -n "${CI_COMMIT_TAG}" ]; then
    NEW_VERSION="${CI_COMMIT_TAG}"
else
    GIT_DESCRIBE=$(git describe --tags --long --match "[0-9]*" 2>/dev/null) || GIT_DESCRIBE=""

    # POSIX sh has no regex capture groups - this format is fixed
    # (<tag>-<commits>-g<sha>, tag itself never contains a dash), so
    # plain pattern removal from front/back is enough to split it.
    case "${GIT_DESCRIBE}" in
        [0-9]*.[0-9]*.[0-9]*-[0-9]*-g*)
            SHA="${GIT_DESCRIBE##*-g}"
            REST="${GIT_DESCRIBE%-g*}"
            COMMITS="${REST##*-}"
            TAG="${REST%-*}"
            if [ "${COMMITS}" = "0" ]; then
                NEW_VERSION="${TAG}"
            else
                NEW_VERSION="${TAG}+git${COMMITS}.g${SHA}"
            fi
            ;;
        *)
            NEW_VERSION="0.0.0+git.$(git log -1 --format=%cd --date=format:%Y%m%d).g$(git rev-parse --short HEAD)"
            ;;
    esac
fi

NEW_LINE="ostserver (${NEW_VERSION}) UNRELEASED; urgency=medium"
echo "Computed debian/changelog version: ${NEW_VERSION}"

if [ "$(head -1 debian/changelog)" != "${NEW_LINE}" ]; then
    # Same job-token push pattern as CHANGELOG.md's own commit (see
    # CI-upload-release-tag.sh) - isolated subshell, a failure here never
    # affects the release/upload already done by the calling script.
    (
        set -e
        git config user.email "gilles@joag.fr"
        git config user.name "gilles"
        git fetch origin main
        git checkout -B main FETCH_HEAD
        { printf '%s\n' "${NEW_LINE}"; tail -n +2 debian/changelog; } > debian/changelog.new
        mv debian/changelog.new debian/changelog
        git add debian/changelog
        git commit -m "debian: sync changelog version to ${NEW_VERSION}"
        git push origin main
    ) || echo "debian/changelog sync failed (non-fatal)"
else
    echo "debian/changelog already up to date, nothing to push"
fi

# A real tag release: point "release" (ostserver-stable's Launchpad
# recipe source) at main's current tip, so that recipe always builds
# exactly the last tag - no manual merge into release needed anymore.
# Force push: release's tip is never a pure ancestor of main's, even
# right after merging main into it - that merge commit itself only
# ever lives on release, so a plain push is rejected as non-fast-forward
# every single time. release has no content of its own to lose - it's
# meant to be a pure mirror now.
if [ -n "${CI_COMMIT_TAG}" ]; then
    (
        set -e
        git config user.email "gilles@joag.fr"
        git config user.name "gilles"
        git fetch origin main
        git push --force origin FETCH_HEAD:release
    ) || echo "release branch sync failed (non-fatal)"
fi
