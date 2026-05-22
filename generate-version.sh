#!/bin/bash
#
# Generate version.cc from git information
# Falls back to debian changelog when building outside a git repository (e.g. Launchpad)
#

OUTPUT_FILE="$1"
TEMPLATE_FILE="$2"

if [ -z "$OUTPUT_FILE" ] || [ -z "$TEMPLATE_FILE" ]; then
    echo "Usage: $0 <output_file> <template_file>"
    exit 1
fi

if git rev-parse --git-dir > /dev/null 2>&1; then
    # Inside a git repository — use git metadata
    GIT_SHA1=$(git describe --match=NeVeRmAtCh --always --abbrev=40 --dirty 2>/dev/null || echo "unknown")
    GIT_DATE=$(git log -1 --format=%ad --date=local 2>/dev/null || echo "unknown")
    GIT_COMMIT_SUBJECT=$(git log -1 --format=%s 2>/dev/null || echo "unknown")
    GIT_TAG=$(git describe --tags --exact-match 2>/dev/null)
    if [ -z "$GIT_TAG" ]; then
        GIT_TAG=$(git log -1 --format=%h 2>/dev/null || echo "unknown")
    fi
else
    # Not a git repository (e.g. Launchpad source tarball) — fall back to debian changelog
    DEB_VERSION=$(dpkg-parsechangelog --show-field Version 2>/dev/null || echo "unknown")
    DEB_DATE=$(dpkg-parsechangelog --show-field Date 2>/dev/null || echo "unknown")
    GIT_SHA1="unknown"
    GIT_DATE="$DEB_DATE"
    GIT_COMMIT_SUBJECT="Launchpad build"
    GIT_TAG="$DEB_VERSION"
fi

escape_sed() { printf '%s\n' "$1" | sed 's/[|&\]/\\&/g'; }

sed -e "s|@GIT_SHA1@|$(escape_sed "${GIT_SHA1}")|g" \
    -e "s|@GIT_DATE@|$(escape_sed "${GIT_DATE}")|g" \
    -e "s|@GIT_COMMIT_SUBJECT@|$(escape_sed "${GIT_COMMIT_SUBJECT}")|g" \
    -e "s|@GIT_TAG@|$(escape_sed "${GIT_TAG}")|g" \
    "$TEMPLATE_FILE" > "$OUTPUT_FILE"

echo "Generated $OUTPUT_FILE"
echo "  GIT_SHA1: $GIT_SHA1"
echo "  GIT_DATE: $GIT_DATE"
echo "  GIT_COMMIT_SUBJECT: $GIT_COMMIT_SUBJECT"
echo "  GIT_TAG: $GIT_TAG"
