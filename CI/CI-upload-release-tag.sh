#!/bin/bash
#
# CI - Upload + Release - TAG
#
set -e
apk add curl
echo "upload packages"
for DEB in ostserver_${CI_COMMIT_TAG}_2404_amd64.deb ostserver_${CI_COMMIT_TAG}_2604_amd64.deb; do
    [ -f "${DEB}" ] && curl --location --header "JOB-TOKEN: ${CI_JOB_TOKEN}" --upload-file "${DEB}" "${PACKAGE_REGISTRY_TAG}/${DEB}"
done
echo "create release"
release-cli create --name "Release $CI_COMMIT_TAG" --tag-name $CI_COMMIT_TAG \
  --assets-link "{\"name\":\"ostserver_${CI_COMMIT_TAG}_2404_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_TAG}/ostserver_${CI_COMMIT_TAG}_2404_amd64.deb\"}" \
  --assets-link "{\"name\":\"ostserver_${CI_COMMIT_TAG}_2604_amd64.deb\",\"url\":\"${PACKAGE_REGISTRY_TAG}/ostserver_${CI_COMMIT_TAG}_2604_amd64.deb\"}"
