#!/bin/bash
#
# CI - Build Hugo documentation site
#
set -e

HUGO_PARAMS_VERSION=$(git rev-parse --short HEAD) hugo --gc --minify
tar -czf ${DOC_FILE_LATEST} -C public/ .
