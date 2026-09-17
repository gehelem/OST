#!/bin/bash
#
# CI - Build Hugo documentation site
#
set -e

HUGO_PARAMS_VERSION=$(git rev-parse --short HEAD) hugo --gc --minify
tar -czf ${DOC_FILE_LATEST} -C public/ .
[ -n "$CI_COMMIT_TAG" ] && cp ${DOC_FILE_LATEST} documentation_${CI_COMMIT_TAG}.tar.gz
