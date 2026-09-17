#!/bin/bash
#
# CI - Build Hugo documentation site
#
set -e

HUGO_PARAMS_VERSION=$(git rev-parse --short HEAD) hugo --gc --minify

# Doxygen API doc (built by the build-doxygen job) rides along as /api/
if [ -d ../doc/html ]; then
    mkdir -p public/api
    cp -r ../doc/html/. public/api/
fi

tar -czf ${DOC_FILE_LATEST} -C public/ .
if [ -n "$CI_COMMIT_TAG" ]; then
    cp ${DOC_FILE_LATEST} documentation_${CI_COMMIT_TAG}.tar.gz
fi
