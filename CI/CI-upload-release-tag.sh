#!/bin/bash
#
# CI - Upload + Release - TAG
#
set -e
apk add curl
echo "upload packages"
curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver_${CI_COMMIT_TAG}_amd64.deb "${PACKAGE_REGISTRY_TAG}/ostserver_${CI_COMMIT_TAG}_amd64.deb"
[ -f ostserver_${CI_COMMIT_TAG}_2604_amd64.deb ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file ostserver_${CI_COMMIT_TAG}_2604_amd64.deb "${PACKAGE_REGISTRY_TAG}/ostserver_${CI_COMMIT_TAG}_2604_amd64.deb"
echo "create release"
ASSETS="--assets-link {\"name\":\"ostserver_${CI_COMMIT_TAG}_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_TAG}/ostserver_${CI_COMMIT_TAG}_amd64.deb\"}"
[ -f ostserver_${CI_COMMIT_TAG}_2604_amd64.deb ] && ASSETS="$ASSETS --assets-link {\"name\":\"ostserver_${CI_COMMIT_TAG}_2604_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_TAG}/ostserver_${CI_COMMIT_TAG}_2604_amd64.deb\"}"
release-cli create --name "Release $CI_COMMIT_TAG" --tag-name $CI_COMMIT_TAG $ASSETS
