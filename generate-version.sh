#!/bin/bash
#
# Generate version.cc from git information
# This script is called automatically during 'make' build
#

OUTPUT_FILE="$1"
TEMPLATE_FILE="$2"

if [ -z "$OUTPUT_FILE" ] || [ -z "$TEMPLATE_FILE" ]; then
    echo "Usage: $0 <output_file> <template_file>"
    exit 1
fi

# Get git information
GIT_SHA1=$(git describe --match=NeVeRmAtCh --always --abbrev=40 --dirty 2>/dev/null || echo "unknown")
GIT_DATE=$(git log -1 --format=%ad --date=local 2>/dev/null || echo "unknown")
GIT_COMMIT_SUBJECT=$(git log -1 --format=%s 2>/dev/null || echo "unknown")

# Replace placeholders in template
sed -e "s|@GIT_SHA1@|${GIT_SHA1}|g" \
    -e "s|@GIT_DATE@|${GIT_DATE}|g" \
    -e "s|@GIT_COMMIT_SUBJECT@|${GIT_COMMIT_SUBJECT}|g" \
    "$TEMPLATE_FILE" > "$OUTPUT_FILE"

echo "Generated $OUTPUT_FILE"
echo "  GIT_SHA1: $GIT_SHA1"
echo "  GIT_DATE: $GIT_DATE"
echo "  GIT_COMMIT_SUBJECT: $GIT_COMMIT_SUBJECT"
